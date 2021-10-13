#include "common.hpp"

using namespace program;

struct candle
{
    std::string event_time;
    float open;
    float close;
    float high;
    float low;
    float volume;

    candle difference(candle& other)
    {
        candle candle;

        candle.event_time = other.event_time;

        candle.open = (this->open == 0) ? 0 : (other.open - this->open) / this->open;
        candle.close = (this->close == 0) ? 0 : (other.close - this->close) / this->close;
        candle.high = (this->high == 0) ? 0 : (other.high - this->high) / this->high;
        candle.low = (this->low == 0) ? 0 : (other.low - this->low) / this->low;
        candle.volume = (this->volume == 0) ? 0 : (other.volume - this->volume) / this->volume;

        return candle;
    }
};

void process_csv(const char* in_file, const char* out)
{
    CSVWriter csv;
    io::CSVReader<6> in(in_file);
    
    in.read_header(io::ignore_missing_column | io::ignore_extra_column, "event_time", "open", "close", "high", "low", "volume");
    csv.newRow() << "event_time" << "open" << "close" << "high" << "low" << "volume";

    std::stack<candle> candles;
    std::stack<candle> processed_candles;

    candle first_candle;
    if (!in.read_row(first_candle.event_time, first_candle.open, first_candle.close, first_candle.high, first_candle.low, first_candle.volume)) return;
    candles.push(first_candle);

    candle next_candle;
    while (in.read_row(next_candle.event_time, next_candle.open, next_candle.close, next_candle.high, next_candle.low, next_candle.volume))
    {
        candle previous = candles.top();
        candles.pop();

        candle change_candle = previous.difference(next_candle);
        processed_candles.push(change_candle);
        
        csv.newRow() << change_candle.event_time << change_candle.open << change_candle.close << change_candle.high << change_candle.low << change_candle.volume;

        candle copy = next_candle;
        candles.push(copy);
    }

    // write to file https://github.com/al-eax/CSVWriter
    char outpath[100];
    strcpy(outpath, out);
    std::filesystem::path path{in_file};
    strcat(outpath, path.filename().string().c_str());
    csv.writeToFile(outpath, true);
}


//void process_csv(const char* in_file, const char* out)
//{
//    CSVWriter csv;
//    io::CSVReader<8, io::trim_chars<' '>, io::no_quote_escape<','>> in(in_file);
//
//    in.read_header(io::ignore_missing_column | io::ignore_extra_column, "event_time", "kline_start_time", "kline_close_time", "open", "close", "high", "low", "volume");
//    csv.newRow() << "event_time" << "kline_start_time" << "kline_close_time" << "open" << "close" << "high" << "low" << "volume";
//
//    std::stack<candle> candles;
//    //std::stack<candle> processed_candles;
//
//    candle first_candle;
//    if (!in.read_row(first_candle.event_time, first_candle.kline_start_time, first_candle.kline_close_time, first_candle.open, first_candle.close, first_candle.high, first_candle.low, first_candle.volume)) return;
//    candles.push(first_candle);
//
//    candle next_candle;
//    while (in.read_row(next_candle.event_time, next_candle.kline_start_time, next_candle.kline_close_time, next_candle.open, next_candle.close, next_candle.high, next_candle.low, next_candle.volume))
//    {
//        candle previous = candles.top();
//        candles.pop();
//
//        candle change_candle = previous.difference(next_candle);
//        //processed_candles.push(change_candle);
//
//        csv.newRow() << change_candle.event_time << change_candle.kline_start_time << change_candle.kline_close_time << change_candle.open << change_candle.close << change_candle.high << change_candle.low << change_candle.volume;
//
//        candle copy = next_candle;
//        candles.push(copy);
//    }
//
//    // write to file https://github.com/al-eax/CSVWriter
//    char outpath[100];
//    strcpy(outpath, out);
//    std::filesystem::path path{ in_file };
//    strcat(outpath, path.filename().string().c_str());
//    csv.writeToFile(outpath, true);
//}

/**
 * @param {number} argc Argument count
 * @param {Array<char*>} argv Array of arguments
 */
int main(int argc, const char** argv)
{
    g_log->info("MAIN", "Initiating thread pool.");
    auto thread_pool_instance = std::make_unique<thread_pool>();

    if (argc < 3)
    {
        g_log->error("MAIN", "Missing arguments, input_folder and/or output_folder");

        return 1;
    }

    const char* input_folder = argv[1];
    const char* output_folder = argv[2];

    if (!std::filesystem::exists(input_folder) || !std::filesystem::exists(output_folder))
    {
        g_log->error("MAIN", "Input and/or output folder do not exist.");

        return 1;
    }

    for (const auto file : std::filesystem::directory_iterator(input_folder))
    {
        // & reference to all vars above
        // = copy all vars above
        g_thread_pool->push([=]()
        {
            g_log->info("THREAD", "Processing file: %s", file.path().string().c_str());

            if (!file.is_directory())
                process_csv(file.path().string().c_str(), output_folder);
        });
    }

    while (thread_pool_instance->has_jobs())
    {
        std::this_thread::sleep_for(500ms);
    }

    thread_pool_instance->destroy();
    thread_pool_instance.reset();

    system("pause");

    return 0;
}