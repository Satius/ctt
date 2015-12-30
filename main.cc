#include "mp3.h"
#include "thread.h"
#include "util.h"
#include "wav.h"
#include <iostream>
#include <vector>

namespace {

const char kDstExtension[] = "mp3";
const char kSrcExtension[] = "wav";
const int kBufferSize = 1024 * 1024;
const int kMp3Quality = 5;

struct Context {
  int counter;
  pthread_mutex_t log_mutex;
  pthread_mutex_t queue_mutex;
  std::stack<std::string> files;

  Context(const std::stack<std::string>& arg) :
      counter(0),
      log_mutex(PTHREAD_MUTEX_INITIALIZER),
      queue_mutex(PTHREAD_MUTEX_INITIALIZER),
      files(arg) {}
};

void PrintFormattedException(const std::exception& ex) {
  int i = 0;
  util::UnwindNested(ex, [&i](const std::exception& op){
    std::cerr << "!" << std::string(i++ * 2 + 1, ' ') << op.what() << std::endl;
  });
}

void HandleStreams(IInputStream& src, IOutputStream& dst) {
  try {
    std::vector<uint8_t> buffer(kBufferSize);
    for (;;) {
      auto size = src.Read(buffer.data(), buffer.size());
      if (!size) break;
      if (dst.Write(buffer.data(), size) != size)
        throw std::runtime_error("Failed to write compete data");
    }
  } catch (const std::exception&) {
    std::throw_with_nested(std::runtime_error("Streams handling failed"));
  }
}

void EncodeFile(const std::string& path) {
  try {
    if (!util::CheckExtension(path, kSrcExtension))
      throw std::runtime_error("Wrong source file extension");
    auto src = wav::ReadWavFile(path);
    const auto& src_info = src->GetInfo();
    mp3::Info dst_info = {src_info.num_channels, src_info.sample_rate, src_info.bits_per_sample, kMp3Quality};
    auto dst = mp3::WriteMp3File(util::ChangeExtension(path, kDstExtension));
    dst->SetInfo(dst_info);
    HandleStreams(*src, *dst);
  } catch (const std::exception&) {
    const auto& msg = util::Join("Failed to encode file", path);
    std::throw_with_nested(std::runtime_error(msg));
  }
}

std::string GetNextFile(Context* ctx) {
  thread::MutexLocker lock(&ctx->queue_mutex);
  if (ctx->files.empty()) return {};
  auto result = ctx->files.top();
  ctx->files.pop();
  return result;
}

int GetThreadId(Context* ctx) {
  thread::MutexLocker locker(&ctx->queue_mutex);
  return ++ctx->counter;
}

void* Worker(void* arg) {
  auto ctx = reinterpret_cast<Context*>(arg);
  auto thread_id = GetThreadId(ctx);
  for (;;) try {
    const auto& fname = GetNextFile(ctx);
    if (fname.empty()) return nullptr;
    const auto& message = util::Join("Handling", fname, "in thread", thread_id);
    {
      thread::MutexLocker locker(&ctx->log_mutex);
      std::cout << message << std::endl;
    }
    EncodeFile(fname);
  } catch (const std::exception& ex) {
    thread::MutexLocker locker(&ctx->log_mutex);
    PrintFormattedException(ex);
  }
}

}  // namespace

int main(int argc, char** argv) {
  try {
    if (argc < 2) throw std::runtime_error("Wrong set of arguments");
    const auto& filter = std::bind(util::CheckExtension, std::placeholders::_1, kSrcExtension);
    std::unique_ptr<Context> ctx(new Context(util::EnumDir(argv[1], filter)));
    if (ctx->files.empty()) {
      std::cout << "No wav files found in directory " << argv[1] << std::endl;
      return 0;
    }
    std::vector<pthread_t> threads(util::GetCoreCount());
    for (auto& it : threads) {
      if (pthread_create(&it, nullptr, Worker, ctx.get()))
        util::ThrowSystemError("Failed to create a thread");
    }
    for (auto it : threads) {
      if (pthread_join(it, nullptr))
        util::ThrowSystemError("Failed to join a thread");
    }
    return 0;
  } catch (const std::exception& ex) {
    PrintFormattedException(ex);
    return 1;
  }
}
