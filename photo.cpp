#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>

//структура rgb
struct Color 
{
    uint8_t r, g, b;
};

using Image = std::vector<std::vector<Color>>;

// генераци€ тестового
Image createImage(int width, int height) 
{
    Image img(height, std::vector<Color>(width));
    for (int y = 0; y < height; ++y) 
    {
        for (int x = 0; x < width; ++x) 
        {
            img[y][x] = 
            {
                static_cast<uint8_t>(rand() % 256),
                static_cast<uint8_t>(rand() % 256),
                static_cast<uint8_t>(rand() % 256)
            };
        }
    }
    return img;
}

// последовательное размытие 3x3
Image sequentialBlur(const Image& input) 
{
    Image output = input;
    int height = input.size();
    int width = input[0].size();

    for (int y = 1; y < height - 1; ++y) 
    {
        for (int x = 1; x < width - 1; ++x) 
        {
            int sum_r = 0, sum_g = 0, sum_b = 0;
            for (int dy = -1; dy <= 1; ++dy) 
            {
                for (int dx = -1; dx <= 1; ++dx) 
                {
                    sum_r += input[y + dy][x + dx].r;
                    sum_g += input[y + dy][x + dx].g;
                    sum_b += input[y + dy][x + dx].b;
                }
            }
            output[y][x] = 
            {
                static_cast<uint8_t>(sum_r / 9),
                static_cast<uint8_t>(sum_g / 9),
                static_cast<uint8_t>(sum_b / 9)
            };
        }
    }
    return output;
}

// параллельное размытие с потоками
void blurPart(const Image& input, Image& output, int startY, int endY) 
{
    int width = input[0].size();
    for (int y = startY; y < endY; ++y) 
    {
        for (int x = 1; x < width - 1; ++x) 
        {
            int sum_r = 0, sum_g = 0, sum_b = 0;
            for (int dy = -1; dy <= 1 ; ++dy) 
            {
                for (int dx = -1; dx <= 1; ++dx) 
                {
                    sum_r += input[y + dy][x + dx].r;
                    sum_g += input[y + dy][x + dx].g;
                    sum_b += input[y + dy][x + dx].b;
                }
            }
            output[y][x] = 
            {
                static_cast<uint8_t>(sum_r / 9),
                static_cast<uint8_t>(sum_g / 9),
                static_cast<uint8_t>(sum_b / 9)
            };
        }
    }
}

Image parallelBlurThreads(const Image& input, int numThreads) 
{
    Image output = input;
    std::vector<std::thread> threads;
    int rowsPerThread = input.size() / numThreads;

    for (int i = 0; i < numThreads; ++i) 
    {
        int startY = i * rowsPerThread;
        int endY = (i == numThreads - 1) ? input.size() : (i + 1) * rowsPerThread;
        threads.emplace_back(blurPart, std::ref(input), std::ref(output), startY + 1 , endY - 1);
    }

    for (auto& thread : threads) 
    {
        thread.join();
    }

    return output;
}

// пример атомарных и мьютексов
void atomicExample() 
{
    std::atomic<int> atomicCounter(0);
    std::mutex mtx;
    int counter = 0;

    auto atomicTask = [&]() 
        {
        for (int i = 0; i < 100000; ++i) 
        {
            atomicCounter++;
        }
        };

    auto mutexTask = [&]() 
        {
        for (int i = 0; i < 100000; ++i) 
        {
            std::lock_guard<std::mutex> lock(mtx);
            counter++;
        }
        };

    // тест атомарной версии
    auto start = std::chrono::high_resolution_clock::now();
    std::thread t1(atomicTask);
    std::thread t2(atomicTask);
    t1.join();
    t2.join();
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Atomic time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " мкс\n";

    // тест мьютекса
    start = std::chrono::high_resolution_clock::now();
    std::thread t3(mutexTask);
    std::thread t4(mutexTask);
    t3.join();
    t4.join();
    end = std::chrono::high_resolution_clock::now();
    std::cout << "Mutex time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " мкс\n";
}

int main() {
    setlocale(LC_ALL, "");
    // изображение 1920x1080
    Image img = createImage(1920, 1080);

    // последовательное размытие
    auto start = std::chrono::high_resolution_clock::now();
    Image blurredSeq = sequentialBlur(img);
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Sequential blur: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " мс\n";

    // ѕараллельное размытие 4 потока
    start = std::chrono::high_resolution_clock::now();
    Image blurredPar = parallelBlurThreads(img, 4);
    end = std::chrono::high_resolution_clock::now();
    std::cout << "Parallel blur (4 threads): " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " мс\n";

    // ѕример атомарных
    atomicExample();

    return 0;
}