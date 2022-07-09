
// @Section: Work Queue
#define WORK_QUEUE_CALLBACK(name) void name(void *data)
typedef WORK_QUEUE_CALLBACK(WorkQueueCallback);

struct WorkQueueEntry {
  WorkQueueCallback *callback;
  void *data;
};

struct WorkQueue {
  WorkQueueEntry entries[256];
  S64 volatile index_to_write;
  S64 volatile index_to_read;
  S64 volatile completion_index;
  S64 volatile completion_goal;
  HANDLE semaphore;
};

struct ThreadStartupInfo {
  DWORD thread_id;
  S32 thread_index;
  WorkQueue *queue;
};

S64 atomic_increment(volatile S64 *i){
  return InterlockedIncrement64(i);
}

S64 atomic_compare_and_swap(volatile S64 *dst, S64 exchange, S64 comperand){
  return InterlockedCompareExchange64(dst, exchange, comperand);
}

void push_work(WorkQueue *wq, void *data, WorkQueueCallback *callback) {
  U32 new_index = (wq->index_to_write + 1) % buff_cap(wq->entries);
  assert(new_index != wq->index_to_read);

  WorkQueueEntry *entry = wq->entries + wq->index_to_write;
  entry->data = data;
  entry->callback = callback;

  wq->completion_goal+=1;
  _WriteBarrier();
  wq->index_to_write = new_index;
  ReleaseSemaphore(wq->semaphore, 1, 0);
}

bool try_doing_work(WorkQueue *wq) {
  bool should_sleep = false;
  S64 original_index_to_read = wq->index_to_read;
  S64 new_index_to_read = (original_index_to_read + 1) % buff_cap(wq->entries);
  if(original_index_to_read != wq->index_to_write) {
    S64 index = atomic_compare_and_swap(&wq->index_to_read, new_index_to_read, original_index_to_read);
    if(index == original_index_to_read) {
      WorkQueueEntry *entry = wq->entries + index;
      entry->callback(entry->data);
      atomic_increment(&wq->completion_index);
    }
  }
  else {
    should_sleep = true;
  }
  return should_sleep;
}

DWORD WINAPI thread_proc(LPVOID param) {
  auto ti = (ThreadStartupInfo *)param;

  Thread_Ctx ctx = {};
  ctx.thread_index = ti->thread_index;
  for(;;) {
    if(try_doing_work(ti->queue)) {
      WaitForSingleObject(ti->queue->semaphore, INFINITE);
    }
  }
}

void init_work_queue(WorkQueue *queue, U32 thread_count, ThreadStartupInfo *info) {
  queue->index_to_read = 0;
  queue->index_to_write = 0;
  queue->completion_index = 0;
  queue->completion_goal = 0;
  queue->semaphore = CreateSemaphoreExA(0, 0, thread_count, 0, 0, SEMAPHORE_ALL_ACCESS);
  assert_msg(queue->semaphore != INVALID_HANDLE_VALUE, "Failed to create semaphore");

  for(U32 i = 0; i < thread_count; i++) {
    ThreadStartupInfo *ti = info + i;
    ti->thread_index = i;
    ti->queue = queue;
    HANDLE thread_handle = CreateThread(0, 0, thread_proc, ti, 0, &ti->thread_id);
    assert_msg(thread_handle != INVALID_HANDLE_VALUE, "Failed to create thread");
    CloseHandle(thread_handle);
  }
}

void wait_until_completion(WorkQueue *wq) {
  while(wq->completion_goal != wq->completion_index) {
    try_doing_work(wq);
  }
}
