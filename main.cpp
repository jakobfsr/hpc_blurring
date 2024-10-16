#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>

void blur_row(const cv::Mat& img_array, cv::Mat& dest_img, int start_row, int end_row) {
    for (int row = start_row; row < end_row; row++) {
        for (int col = 0; col < img_array.cols; col++) {
            cv::Vec3i sum(0, 0, 0);
            int fields = 0;

            for (int i = -1; i <= 1; i++) {
                for (int j = -1; j <= 1; j++) {
                    int r = row + i, c = col + j;
                    if (r >= 0 && r < img_array.rows && c >= 0 && c < img_array.cols) {
                        sum += img_array.at<cv::Vec3b>(r, c);
                        fields++;
                    }
                }
            }

            dest_img.at<cv::Vec3b>(row, col) = sum / fields;
        }
    }
}

cv::Mat blur_array(const cv::Mat& img_array) {
    cv::Mat dest_img = cv::Mat::zeros(img_array.size(), img_array.type());
    int num_threads = std::thread::hardware_concurrency(); // Anzahl der verfügbaren Threads
    std::vector<std::thread> threads;
    int rows_per_thread = img_array.rows / num_threads;

    // Threads erzeugen und parallel Zeilen bearbeiten
    for (int t = 0; t < num_threads; t++) {
        int start_row = t * rows_per_thread;
        int end_row = (t == num_threads - 1) ? img_array.rows : start_row + rows_per_thread;
        threads.emplace_back(blur_row, std::cref(img_array), std::ref(dest_img), start_row, end_row);
    }

    // Warten, bis alle Threads fertig sind
    for (auto& th : threads) {
        th.join();
    }

    return dest_img;
}

void blur(const std::string& orig_path, int steps) {
    auto runtime_total = std::chrono::high_resolution_clock::now();
    auto last_iteration = runtime_total;

    cv::Mat img = cv::imread(orig_path);
    if (img.empty()) {
        std::cerr << "Error loading image" << std::endl;
        return;
    }

    std::vector<double> runtime_per_iteration;

    for (int i = 0; i < steps; i++) {
        img = blur_array(img);
        auto current_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = current_time - last_iteration;
        runtime_per_iteration.push_back(elapsed.count());
        last_iteration = current_time;

        std::cout << i + 1 << " Steps finished, " << runtime_per_iteration.back() << " seconds" << std::endl;
    }

    size_t idx = orig_path.find_last_of(".");
    std::string output_path = orig_path.substr(0, idx) + "_blur_" + std::to_string(steps) + "_steps" + orig_path.substr(idx);
    cv::imwrite(output_path, img);

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> total_elapsed = end_time - runtime_total;
    std::cout << "Total Runtime: " << total_elapsed.count() << " seconds" << std::endl;
}

int main() {
    blur("/Users/jakobfischer/CLionProjects/untitled2/image-to-blur.jpg", 5);
    return 0;
}
