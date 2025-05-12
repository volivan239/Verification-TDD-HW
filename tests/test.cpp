#include <gtest/gtest.h>
#include <pthread.h>
#include "storage.h"
#include "thread_tasks.h"

TEST(SingleThreadStorageTest, SimpleLoadStoreTest) {
  Storage<std::string, int> storage(1000, 10);

  storage.store("hello", 0);
  storage.store("world", 1);

  EXPECT_EQ(0, storage.load("hello"));
  EXPECT_EQ(1, storage.load("world"));
}

TEST(SingleThreadStorageTest, OverwriteTest) {
  Storage<std::string, int> storage(1000, 10);

  storage.store("hello", 2);
  storage.store("hello", 3);
  storage.store("hello", 9);

  EXPECT_EQ(9, storage.load("hello"));
}

TEST(SingleThreadStorageTest, NoSuchElementTest) {
  Storage<std::string, int> storage(1000, 10);

  storage.store("hello", 2);

  EXPECT_THROW(storage.load("world"), NoSuchElementException);
}

TEST(SingleThreadStorageTest, StorageOverflowTest) {
  Storage<std::string, int> storage(2, 1);

  storage.store("hello", 2);
  storage.store("world", 3);

  EXPECT_NO_THROW(storage.store("!", 4)); // This should be written to cache
  EXPECT_THROW(storage.store("!!", 5), StorageOverflowException); // Exception thrown at cache flushing
}

TEST(SingleThreadStorageTest, DefaultValueKeyTest) {
  Storage<std::string, int> storage(2, 2);

  EXPECT_THROW(storage.load(""), NoSuchElementException);
}

TEST(SingleThreadStorageTest, UpdateIfEqualsTest) {
  Storage<std::string, int> storage(2, 1);

  storage.store("abc", 0);
  storage.store("def", 0); // store to flush "abc" from cache
  EXPECT_EQ(storage.updateIfEquals("abc", 0, 1), true);
  EXPECT_EQ(storage.updateIfEquals("abc", 0, 2), false);
  EXPECT_EQ(storage.updateIfEquals("", 0, 0), false);
  EXPECT_EQ(storage.load("abc"), 1);
}


TEST(MultiThreadedTest, MultiThreadedLoad) {
  const int N = 20000;
  pthread_t threads[N];

  Storage<int, int> storage(2, 1);
  storage.store(0, 0);
  storage.store(1, 1);
  
  LoadWithCheckTask<int, int> tasks[N];
  for (int i = 0; i < N; i++) {
    tasks[i] = {&storage, i & 1, i & 1};
    pthread_create(threads + i, NULL, loadWithCheck<int, int>, tasks + i);
  }
  for (int i = 0; i < N; i++) {
    pthread_join(threads[i], NULL);
  }
}

TEST(MultiThreadedTest, MultiThreadedStore) {
  const int N = 20000;
  pthread_t threads[N];

  Storage<int, int> storage(2, 1);

  StoreNoExceptTask<int, int> tasks[N];
  for (int i = 0; i < N; i++) {
    tasks[i] = {&storage, i & 1, i & 1};
    pthread_create(threads + i, NULL, storeNoExcept<int, int>, tasks + i);
  }
  for (int i = 0; i < N; i++) {
    pthread_join(threads[i], NULL);
  }
  EXPECT_EQ(storage.load(0), 0);
  EXPECT_EQ(storage.load(1), 1);
}

TEST(MultiThreadedTest, MultiThreadedStoreDifferentKeysHeavy) {
  const int N = 10000;
  pthread_t threads[N];

  Storage<long long, int> storage(N, 100);

  StoreNoExceptTask<long long, int> tasks[N];
  for (int i = 0; i < N; i++) {
    tasks[i] = {&storage, 1ll * i * i, i};
    pthread_create(threads + i, NULL, storeNoExcept<long long, int>, tasks + i);
  }
  for (int i = 0; i < N; i++) {
    pthread_join(threads[i], NULL);
  }
  for (int i = 0; i < N; i++) {
    EXPECT_EQ(storage.load(1ll * i * i), i);
  }
  EXPECT_THROW(storage.load(2), NoSuchElementException);
}

TEST(MultiThreadedTest, MultiThreadedUpdateIfEqualsHeavy) {
  const int N = 10000;
  pthread_t threads[N];

  Storage<int, int> storage(2, 1);

  UpdateIfEqualsTask<int, int> tasks[N];
  storage.store(0, 0);
  storage.store(2, 0);

  for (int i = 0; i < N; i++) {
    tasks[i] = {&storage, i & 2, i & 1, 1 - (i & 1), false};
    pthread_create(threads + i, NULL, updateIfEquals<int, int>, tasks + i);
  }
  for (int i = 0; i < N; i++) {
    pthread_join(threads[i], NULL);
  }

  for (int key = 0; key <= 2; key += 2) {
    int success_0_to_1 = 0;
    int success_1_to_0 = 0;

    for (int i = 0; i < N; i++) {
      if ((i & 2) == key && tasks[i].result) {
        if (i & 1) {
          success_1_to_0++;
        } else {
          success_0_to_1++;
        }
      }
    }
    EXPECT_EQ(success_1_to_0 - success_0_to_1, storage.load(key));
    EXPECT_GE(success_0_to_1 + success_1_to_0, N / 100);
  }
}