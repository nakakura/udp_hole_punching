#include <gtest/gtest.h>
#include <uv.h>

#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "udp_socket.h"

/**
 * UDPSocketクラスのテスト
 */
class UDPSocketTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // テスト前の準備

    // libuvループの初期化状態を確認
    // (UDPSocketの内部でuv_default_loop()を使用するため)
    test_start_time_ = std::chrono::steady_clock::now();

    // 使用済みポートのインデックス
    operation_count_ = 0;
  }

  void TearDown() override {
    // テスト後のクリーンアップ

    // テスト実行時間の記録（デバッグ用）
    auto test_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - test_start_time_);

    // 長時間実行されたテストを警告（デッドロック検出の補助）
    if (test_duration.count() > 5000)  // 5秒以上
    {
      std::cerr << "Warning: Test took " << test_duration.count()
                << "ms (>5s), potential deadlock or performance issue"
                << std::endl;
    }

    // libuvのイベントループを一度実行してリソースをクリーンアップ
    // (未処理のハンドルやリクエストをクリーンアップ)
    uv_run(uv_default_loop(), UV_RUN_NOWAIT);

    // 小さな待機時間でlibuvのクリーンアップを確実にする
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // 最終的なループ実行でクリーンアップ完了
    uv_run(uv_default_loop(), UV_RUN_NOWAIT);
  }

  // テスト用のヘルパー変数
  std::chrono::steady_clock::time_point test_start_time_;
  std::atomic<int> operation_count_{0};

  // テスト用のヘルパーメソッド

  /**
   * テスト用の一意なポートを取得
   */
  int getUniquePort() {
    auto socket = std::make_unique<UDPSocket>();
    if (socket->IsValid()) {
      int port = socket->GetLocalPort();
      if (port > 0) {
        return port;
      }  // socket開放はRAIIに任せる
    }

    return -1;  // エラー：ポート取得失敗
  }

  /**
   * 期待される数のパケットが受信されるまで待機
   */
  template <typename Container>
  bool waitForPackets(
      Container& received_data, std::mutex& data_mutex, size_t expected_count,
      std::chrono::milliseconds timeout = std::chrono::milliseconds(3000)) {
    auto start_wait = std::chrono::steady_clock::now();

    while (std::chrono::steady_clock::now() - start_wait < timeout) {
      {
        std::lock_guard<std::mutex> lock(data_mutex);
        if (received_data.size() >= expected_count) {
          return true;  // 期待される数のパケットを受信
        }
      }
      std::this_thread::sleep_for(
          std::chrono::milliseconds(10));  // 短い間隔でチェック
    }

    return false;  // タイムアウト
  }
};

/**
 * UDPソケットが正常に作成できることを確認
 */
TEST_F(UDPSocketTest, CanCreateUDPSocket) {
  UDPSocket socket;

  // ソケットが正常に作成されていることを確認
  EXPECT_TRUE(socket.IsValid());
}

/**
 * UDPパケットの送信機能をテスト
 */
TEST_F(UDPSocketTest, CanSendUDPPacket) {
  UDPSocket socket;
  ASSERT_TRUE(socket.IsValid());

  // テストデータ
  std::vector<uint8_t> test_data = {'H', 'e', 'l', 'l', 'o'};

  // 一意なポートに送信テスト（ポート競合を避ける）
  int test_port = getUniquePort();
  ASSERT_GT(test_port, 0) << "Failed to get unique port for testing";
  bool result = socket.SendTo(test_data, "127.0.0.1", test_port);
  EXPECT_TRUE(result);
}

/**
 * UDPパケットの受信機能をテスト
 */
TEST_F(UDPSocketTest, CanReceiveUDPPacket) {
  UDPSocket receiver;
  UDPSocket sender;

  ASSERT_TRUE(receiver.IsValid());
  ASSERT_TRUE(sender.IsValid());

  // 受信者のポート番号を取得
  int receiver_port = receiver.GetLocalPort();
  ASSERT_GT(receiver_port, 0);

  // テストデータ
  std::vector<uint8_t> test_data = {'T', 'e', 's', 't'};

  // 送信
  bool send_result = sender.SendTo(test_data, "127.0.0.1", receiver_port);
  EXPECT_TRUE(send_result);

  // 受信（タイムアウト付き）
  std::vector<uint8_t> received_data;
  std::string sender_ip;
  int sender_port;

  bool receive_result = receiver.ReceiveFrom(
      received_data, sender_ip, sender_port, 1000);  // 1秒タイムアウト
  EXPECT_TRUE(receive_result);
  EXPECT_EQ(test_data, received_data);
}

/**
 * 複数パケットの送受信データ整合性テスト
 */
TEST_F(UDPSocketTest, MultiPacketDataIntegrityTest) {
  UDPSocket receiver;
  UDPSocket sender;

  ASSERT_TRUE(receiver.IsValid());
  ASSERT_TRUE(sender.IsValid());

  int receiver_port = receiver.GetLocalPort();
  ASSERT_GT(receiver_port, 0);

  // 異なるサイズ・内容のテストデータを準備
  std::vector<std::vector<uint8_t>> test_packets = {
      {'S', 'm', 'a', 'l', 'l'},  // 小さなパケット
      {'M', 'e', 'd', 'i', 'u', 'm', ' ', 's', 'i', 'z', 'e', 'd'},  // 中サイズ
      {'L', 'a', 'r', 'g', 'e', 'r', ' ', 'p', 'a', 'c', 'k', 'e',
       't'},                                         // 大きなパケット
      {0x00, 0xFF, 0x7F, 0x80, 0x01},                // バイナリデータ
      {'U', 'T', 'F', '8', ' ', 'T', 'e', 's', 't'}  // ASCII文字のテスト
  };

  std::vector<std::vector<uint8_t>> received_packets;

  // 全パケットを送信
  for (size_t i = 0; i < test_packets.size(); ++i) {
    bool send_result =
        sender.SendTo(test_packets[i], "127.0.0.1", receiver_port);
    EXPECT_TRUE(send_result) << "Failed to send packet " << i;

    // 送信間隔を空ける
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  // 送信完了後、全パケットを受信
  size_t max_expected = test_packets.size();
  for (size_t i = 0; i < max_expected; ++i) {
    std::vector<uint8_t> received_data;
    std::string sender_ip;
    int sender_port;

    bool receive_result =
        receiver.ReceiveFrom(received_data, sender_ip, sender_port, 1000);
    if (receive_result && !received_data.empty()) {
      received_packets.push_back(received_data);
    }
  }

  // 少なくとも一部のパケットは受信されているはず
  EXPECT_GT(received_packets.size(), 0) << "No packets were received";

  // 受信したパケットが送信したパケットのいずれかと一致することを確認
  // (UDPで到着順序が保証されないため)
  for (const auto& received_packet : received_packets) {
    bool found_match = false;
    for (const auto& sent_packet : test_packets) {
      if (received_packet == sent_packet) {
        found_match = true;
        break;
      }
    }
    EXPECT_TRUE(found_match)
        << "Received packet that doesn't match any sent packet";
  }

  // 各送信パケットが正確に1回受信されているかをチェック
  if (received_packets.size() == test_packets.size()) {
    // 全パケットが受信された場合の詳細検証
    for (const auto& sent_packet : test_packets) {
      bool found_in_received = false;
      for (const auto& received_packet : received_packets) {
        if (sent_packet == received_packet) {
          found_in_received = true;
          break;
        }
      }
      EXPECT_TRUE(found_in_received)
          << "A sent packet was not found in received packets";
    }
  }
}

/**
 * 複数スレッドからの同時送信テスト：送信内容の受信確認付き
 */
TEST_F(UDPSocketTest, ConcurrentSendTest) {
  // 受信用ソケットを作成
  UDPSocket receiver;
  ASSERT_TRUE(receiver.IsValid());
  int receiver_port = receiver.GetLocalPort();
  ASSERT_GT(receiver_port, 0);

  // 送信用ソケット
  UDPSocket sender;
  ASSERT_TRUE(sender.IsValid());

  const int num_threads = 4;
  const int sends_per_thread = 5;
  std::vector<std::thread> threads;
  std::vector<bool> send_results(num_threads * sends_per_thread, false);
  std::vector<std::vector<uint8_t>> sent_data;  // 送信したデータを記録
  std::mutex sent_data_mutex;

  // 複数スレッドで同時送信
  for (int t = 0; t < num_threads; ++t) {
    threads.emplace_back([this, &sender, &send_results, &sent_data,
                          &sent_data_mutex, t, sends_per_thread,
                          receiver_port]() {
      for (int i = 0; i < sends_per_thread; ++i) {
        std::vector<uint8_t> data = {
            'T',
            'h',
            'r',
            'e',
            'a',
            'd',
            static_cast<uint8_t>('0' + t),  // スレッド番号
            static_cast<uint8_t>('0' + i)   // 送信番号
        };

        bool result = sender.SendTo(data, "127.0.0.1", receiver_port);
        send_results[t * sends_per_thread + i] = result;

        // 送信したデータを記録
        if (result) {
          std::lock_guard<std::mutex> lock(sent_data_mutex);
          sent_data.push_back(data);
        }

        std::this_thread::sleep_for(
            std::chrono::milliseconds(10));  // 少し間隔を空ける
      }
    });
  }

  // 受信スレッド：送信されたパケットを受信して検証
  std::vector<std::vector<uint8_t>> received_data;
  std::mutex received_data_mutex;
  std::atomic<bool> stop_receiving(false);

  std::thread receiver_thread(
      [&receiver, &received_data, &received_data_mutex, &stop_receiving]() {
        while (!stop_receiving.load()) {
          std::vector<uint8_t> packet_data;
          std::string sender_ip;
          int sender_port;

          bool received = receiver.ReceiveFrom(
              packet_data, sender_ip, sender_port, 100);  // 100ms timeout
          if (received && !packet_data.empty()) {
            std::lock_guard<std::mutex> lock(received_data_mutex);
            received_data.push_back(packet_data);
          }
        }
      });

  // 全送信スレッド完了を待機
  for (auto& thread : threads) {
    thread.join();
  }

  // 送信完了後、期待される数のパケットが受信されるまで待機
  const size_t expected_packets = num_threads * sends_per_thread;
  bool received_all =
      waitForPackets(received_data, received_data_mutex, expected_packets);

  stop_receiving.store(true);
  receiver_thread.join();

  // パケット受信の検証を追加
  if (!received_all) {
    std::cout
        << "Warning: Did not receive all expected packets within timeout. "
        << "Expected: " << expected_packets
        << ", Received: " << received_data.size() << std::endl;
  }

  // 結果検証
  // 1. 全ての送信が成功したことを確認
  for (bool result : send_results) {
    EXPECT_TRUE(result) << "Concurrent send failed";
  }

  // 2. 送信したデータが受信されていることを確認
  EXPECT_EQ(sent_data.size(), num_threads * sends_per_thread)
      << "Not all data was recorded as sent";
  EXPECT_GT(received_data.size(), 0) << "No data was received";

  // 受信したパケットが送信したパケットと一致するかを確認
  for (const auto& received_packet : received_data) {
    bool found_match = false;
    for (const auto& sent_packet : sent_data) {
      if (received_packet == sent_packet) {
        found_match = true;
        break;
      }
    }
    EXPECT_TRUE(found_match)
        << "Received packet that doesn't match any sent packet";
  }
}

/**
 * デッドロック検出テスト：同一オブジェクトへの混合操作
 */
TEST_F(UDPSocketTest, DeadlockDetectionMixedOperations) {
  UDPSocket socket;
  ASSERT_TRUE(socket.IsValid());
  int socket_port = socket.GetLocalPort();

  const int num_threads = 6;  // 受信確認のためスレッド数を調整
  const int operations_per_thread = 15;
  std::vector<std::thread> threads;
  std::atomic<int> completed_operations(0);
  std::atomic<bool> deadlock_detected(false);

  // 送信したデータを記録
  std::vector<std::vector<uint8_t>> sent_packets;
  std::vector<std::vector<uint8_t>> received_packets;
  std::mutex data_mutex;

  // タイムアウト監視スレッド（より短い時間で実際に検証）
  std::atomic<bool> test_should_continue(true);
  std::thread timeout_thread([&]() {
    auto start_time = std::chrono::steady_clock::now();
    while (test_should_continue.load()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
          std::chrono::steady_clock::now() - start_time);

      // 3秒でタイムアウト
      if (elapsed.count() >= 3 &&
          completed_operations.load() < num_threads * operations_per_thread) {
        deadlock_detected.store(true);
        break;
      }
    }
  });

  // 複数スレッドで混合操作（送信・受信・状態確認）
  for (int t = 0; t < num_threads; ++t) {
    threads.emplace_back([&socket, &completed_operations, &sent_packets,
                          &received_packets, &data_mutex, t,
                          operations_per_thread, socket_port]() {
      for (int i = 0; i < operations_per_thread; ++i) {
        switch (i % 4)  // 4種類の操作に変更
        {
          case 0:  // 送信（内容を記録）
          {
            std::vector<uint8_t> data = {'D',
                                         'L',
                                         'T',
                                         'e',
                                         's',
                                         't',
                                         static_cast<uint8_t>('0' + t),
                                         static_cast<uint8_t>('0' + i)};
            bool sent = socket.SendTo(data, "127.0.0.1", socket_port);
            if (sent) {
              std::lock_guard<std::mutex> lock(data_mutex);
              sent_packets.push_back(data);
            }
            break;
          }
          case 1:  // 状態確認
          {
            socket.IsValid();
            socket.GetLocalPort();
            break;
          }
          case 2:  // 受信（内容を記録）
          {
            std::vector<uint8_t> received_data;
            std::string sender_ip;
            int sender_port;
            bool received = socket.ReceiveFrom(received_data, sender_ip,
                                               sender_port, 5);  // 5ms短時間
            if (received && !received_data.empty()) {
              std::lock_guard<std::mutex> lock(data_mutex);
              received_packets.push_back(received_data);
            }
            break;
          }
          case 3:  // 別のポートに送信
          {
            std::vector<uint8_t> data = {'E', 'x', 't',
                                         static_cast<uint8_t>('0' + t)};
            socket.SendTo(data, "127.0.0.1", 25000 + t);
            break;
          }
        }
        completed_operations.fetch_add(1);
        std::this_thread::sleep_for(
            std::chrono::microseconds(200));  // 少し時間を増やす
      }
    });
  }

  // 全スレッド完了を待機
  for (auto& thread : threads) {
    thread.join();
  }

  // タイムアウトスレッドを停止
  test_should_continue.store(false);
  timeout_thread.join();

  // デッドロックが発生していないことを確認
  EXPECT_FALSE(deadlock_detected.load())
      << "Deadlock detected in mixed operations";
  EXPECT_EQ(completed_operations.load(), num_threads * operations_per_thread)
      << "Not all operations completed";

  // 少なくともいくつかのパケットが送受信されていることを確認
  EXPECT_GT(sent_packets.size(), 0) << "No packets were sent";

  // 受信したパケットが送信したパケットの中に含まれていることを確認
  for (const auto& received_packet : received_packets) {
    bool found_match = false;
    for (const auto& sent_packet : sent_packets) {
      if (received_packet == sent_packet) {
        found_match = true;
        break;
      }
    }
    EXPECT_TRUE(found_match) << "Received packet that doesn't match any sent "
                                "packet in mixed operations";
  }
}

/**
 * ネストしたロック操作のテスト（潜在的なデッドロック）
 */
TEST_F(UDPSocketTest, DeadlockDetectionNestedCalls) {
  UDPSocket socket1;
  UDPSocket socket2;
  ASSERT_TRUE(socket1.IsValid());
  ASSERT_TRUE(socket2.IsValid());

  std::atomic<bool> deadlock_detected(false);
  std::atomic<int> completed_calls(0);
  const int expected_calls = 100;

  // タイムアウト監視
  std::atomic<bool> test_should_continue(true);
  std::thread timeout_thread([&]() {
    auto start_time = std::chrono::steady_clock::now();
    while (test_should_continue.load()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
      auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::steady_clock::now() - start_time);

      // 1秒でタイムアウト
      if (elapsed.count() >= 1000 && completed_calls.load() < expected_calls) {
        deadlock_detected.store(true);
        break;
      }
    }
  });

  // Thread 1: socket1 -> socket2の順でアクセス
  std::thread thread1([&]() {
    for (int i = 0; i < 50; ++i) {
      std::vector<uint8_t> data = {'S', '1', 'T', 'o', 'S', '2'};
      socket1.SendTo(data, "127.0.0.1", socket2.GetLocalPort());
      completed_calls.fetch_add(1);
      std::this_thread::sleep_for(std::chrono::microseconds(50));
    }
  });

  // Thread 2: socket2 -> socket1の順でアクセス
  std::thread thread2([&]() {
    for (int i = 0; i < 50; ++i) {
      std::vector<uint8_t> data = {'S', '2', 'T', 'o', 'S', '1'};
      socket2.SendTo(data, "127.0.0.1", socket1.GetLocalPort());
      completed_calls.fetch_add(1);
      std::this_thread::sleep_for(std::chrono::microseconds(50));
    }
  });

  thread1.join();
  thread2.join();

  // タイムアウトスレッドを停止
  test_should_continue.store(false);
  timeout_thread.join();

  // デッドロックが発生していないことを確認
  EXPECT_FALSE(deadlock_detected.load()) << "Deadlock detected in nested calls";
  EXPECT_EQ(completed_calls.load(), expected_calls)
      << "Not all nested calls completed";
}

/**
 * 高負荷ストレステスト：デッドロック・リソース枯渇検出
 */
TEST_F(UDPSocketTest, DeadlockStressTest) {
  const int num_sockets = 4;
  const int num_threads_per_socket = 4;
  const int operations_per_thread = 50;

  std::vector<std::unique_ptr<UDPSocket>> sockets;
  std::vector<std::thread> threads;
  std::atomic<int> total_operations(0);
  std::atomic<bool> stress_test_failed(false);

  // 複数ソケットを作成
  for (int i = 0; i < num_sockets; ++i) {
    auto socket = std::make_unique<UDPSocket>();
    ASSERT_TRUE(socket->IsValid()) << "Socket " << i << " creation failed";
    sockets.push_back(std::move(socket));
  }

  // タイムアウト監視
  std::atomic<bool> test_should_continue(true);
  std::thread timeout_thread([&]() {
    auto start_time = std::chrono::steady_clock::now();
    while (test_should_continue.load()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
          std::chrono::steady_clock::now() - start_time);

      int expected =
          num_sockets * num_threads_per_socket * operations_per_thread;
      if (elapsed.count() >= 3 &&
          total_operations.load() < expected * 0.9)  // 3秒でタイムアウト
      {
        stress_test_failed.store(true);
        break;
      }
    }
  });

  // 各ソケットに対して複数スレッドで操作
  for (int s = 0; s < num_sockets; ++s) {
    for (int t = 0; t < num_threads_per_socket; ++t) {
      threads.emplace_back([&, s, t]() {
        auto& socket = sockets[s];

        for (int op = 0; op < operations_per_thread; ++op) {
          try {
            switch (op % 4) {
              case 0:  // Cross-socket communication
              {
                int target_socket = (s + 1) % num_sockets;
                std::vector<uint8_t> data = {
                    'S', static_cast<uint8_t>('0' + s), '>',
                    static_cast<uint8_t>('0' + target_socket)};
                socket->SendTo(data, "127.0.0.1",
                               sockets[target_socket]->GetLocalPort());
                break;
              }
              case 1:  // Status check
              {
                socket->IsValid();
                socket->GetLocalPort();
                break;
              }
              case 2:  // Self-send
              {
                std::vector<uint8_t> data = {'S', 'e', 'l', 'f',
                                             static_cast<uint8_t>('0' + s)};
                socket->SendTo(data, "127.0.0.1", socket->GetLocalPort());
                break;
              }
              case 3:  // Quick receive attempt
              {
                std::vector<uint8_t> received_data;
                std::string sender_ip;
                int sender_port;
                socket->ReceiveFrom(received_data, sender_ip, sender_port,
                                    1);  // 1ms
                break;
              }
            }
            total_operations.fetch_add(1);
          } catch (...) {
            // 例外が発生してもテストを継続
            stress_test_failed.store(true);
          }

          // 負荷を調整
          if (op % 10 == 0) {
            std::this_thread::sleep_for(std::chrono::microseconds(100));
          }
        }
      });
    }
  }

  // 全スレッド完了を待機
  for (auto& thread : threads) {
    thread.join();
  }

  // タイムアウトスレッドを停止
  test_should_continue.store(false);
  timeout_thread.join();

  // ストレステスト結果を確認
  int expected_operations =
      num_sockets * num_threads_per_socket * operations_per_thread;
  EXPECT_FALSE(stress_test_failed.load())
      << "Stress test failed due to timeout or exceptions";
  EXPECT_GE(total_operations.load(), expected_operations * 0.9)
      << "Less than 90% of operations completed. Completed: "
      << total_operations.load() << ", Expected: " << expected_operations;
}

/**
 * 実際の時間測定付きスレッドテスト（受信確認付き）
 */
TEST_F(UDPSocketTest, TimedConcurrentTest) {
  UDPSocket receiver;
  UDPSocket sender;
  ASSERT_TRUE(receiver.IsValid());
  ASSERT_TRUE(sender.IsValid());

  int receiver_port = receiver.GetLocalPort();

  const int num_threads = 3;  // 受信処理を考慮してスレッド数を調整
  const int operations_per_thread = 4;
  const int sleep_ms_per_operation = 50;  // 50ms wait per operation

  auto start_time = std::chrono::steady_clock::now();

  std::vector<std::thread> threads;
  std::vector<bool> results(num_threads * operations_per_thread, false);
  std::vector<std::vector<uint8_t>> sent_data;
  std::mutex sent_data_mutex;

  // 複数スレッドで時間のかかる操作
  for (int t = 0; t < num_threads; ++t) {
    threads.emplace_back([&sender, &results, &sent_data, &sent_data_mutex, t,
                          operations_per_thread, sleep_ms_per_operation,
                          receiver_port]() {
      for (int i = 0; i < operations_per_thread; ++i) {
        std::vector<uint8_t> data = {'T',
                                     'i',
                                     'm',
                                     'e',
                                     'd',
                                     static_cast<uint8_t>('0' + t),
                                     static_cast<uint8_t>('0' + i)};

        bool result = sender.SendTo(data, "127.0.0.1", receiver_port);
        results[t * operations_per_thread + i] = result;

        if (result) {
          std::lock_guard<std::mutex> lock(sent_data_mutex);
          sent_data.push_back(data);
        }

        // 明示的に時間を消費
        std::this_thread::sleep_for(
            std::chrono::milliseconds(sleep_ms_per_operation));
      }
    });
  }

  // 受信スレッド
  std::vector<std::vector<uint8_t>> received_data;
  std::mutex received_data_mutex;
  std::atomic<bool> stop_receiving(false);

  std::thread receiver_thread(
      [&receiver, &received_data, &received_data_mutex, &stop_receiving]() {
        while (!stop_receiving.load()) {
          std::vector<uint8_t> packet_data;
          std::string sender_ip;
          int sender_port;

          bool received =
              receiver.ReceiveFrom(packet_data, sender_ip, sender_port, 100);
          if (received && !packet_data.empty()) {
            std::lock_guard<std::mutex> lock(received_data_mutex);
            received_data.push_back(packet_data);
          }
        }
      });

  // 全スレッド完了を待機
  for (auto& thread : threads) {
    thread.join();
  }

  auto end_time = std::chrono::steady_clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
      end_time - start_time);

  // 送信完了後、期待される数のパケットが受信されるまで待機
  const size_t expected_packets = num_threads * operations_per_thread;
  bool received_all =
      waitForPackets(received_data, received_data_mutex, expected_packets);

  stop_receiving.store(true);
  receiver_thread.join();

  // パケット受信の検証を追加
  if (!received_all) {
    std::cout
        << "Warning: Did not receive all expected packets within timeout. "
        << "Expected: " << expected_packets
        << ", Received: " << received_data.size() << std::endl;
  }

  // 期待される最小時間：operations_per_thread *
  // sleep_ms_per_operation（並列実行なので）
  int expected_min_time = operations_per_thread * sleep_ms_per_operation;

  EXPECT_GE(elapsed.count(), expected_min_time * 0.8)
      << "Test completed too quickly. Expected at least "
      << expected_min_time * 0.8 << "ms but took " << elapsed.count() << "ms";

  // 全ての送信が成功したことを確認
  for (bool result : results) {
    EXPECT_TRUE(result) << "Timed concurrent send failed";
  }

  // 受信データの検証
  EXPECT_GT(received_data.size(), 0) << "No data was received in timed test";

  // 受信したデータが送信したデータと一致することを確認
  for (const auto& received_packet : received_data) {
    bool found_match = false;
    for (const auto& sent_packet : sent_data) {
      if (received_packet == sent_packet) {
        found_match = true;
        break;
      }
    }
    EXPECT_TRUE(found_match)
        << "Received packet that doesn't match any sent packet in timed test";
  }
}

/**
 * タイムアウト検出機能のテスト：わざと無限ループを作ってタイムアウトを確認
 */
TEST_F(UDPSocketTest, TimeoutDetectionTest) {
  std::atomic<bool> deadlock_detected(false);
  std::atomic<int> completed_operations(0);
  const int target_operations = 10;

  // タイムアウト監視（500ms）
  std::atomic<bool> test_should_continue(true);
  std::thread timeout_thread([&]() {
    auto start_time = std::chrono::steady_clock::now();
    while (test_should_continue.load()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
      auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::steady_clock::now() - start_time);

      // 500msでタイムアウト
      if (elapsed.count() >= 500 &&
          completed_operations.load() < target_operations) {
        deadlock_detected.store(true);
        break;
      }
    }
  });

  // わざと途中で止まるスレッド（3回だけ実行して停止）
  std::thread worker_thread([&]() {
    for (int i = 0; i < 3; ++i)  // target_operationsより少ない回数
    {
      completed_operations.fetch_add(1);
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    // ここで意図的に処理を停止（無限待機）
    std::this_thread::sleep_for(std::chrono::seconds(2));
    // 残りの操作はタイムアウト後に実行
    while (completed_operations.load() < target_operations &&
           !deadlock_detected.load()) {
      completed_operations.fetch_add(1);
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  });

  worker_thread.join();

  // タイムアウトスレッドを停止
  test_should_continue.store(false);
  timeout_thread.join();

  // タイムアウト検出が正常に動作したことを確認
  EXPECT_TRUE(deadlock_detected.load())
      << "Timeout detection did not work properly";
  EXPECT_LT(completed_operations.load(), target_operations)
      << "All operations completed unexpectedly";
}
