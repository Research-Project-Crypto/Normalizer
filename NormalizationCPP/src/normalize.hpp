#pragma once
#include "common.hpp"
#include "candle.hpp"

namespace program
{
    class normalizer final
    {
    private:
        std::filesystem::path m_input_file;
        const char* m_out_dir;

        std::vector<std::unique_ptr<candle>> m_candles;

    public:
        normalizer(std::filesystem::path file_path, const char* out_dir) :
            m_input_file(file_path), m_out_dir(out_dir)
        {

        }
        virtual ~normalizer()
        {

        }

        const char* const file_name()
        {
            return m_input_file.filename().c_str();
        }

        void clone_candle(candle* src, candle* dst)
        {
            memcpy(src, dst, sizeof(candle));
        }

        void read_binary_input()
        {
            std::ifstream file(m_input_file.c_str(), std::ios::in | std::ios::binary);

            // read file size
            file.ignore(std::numeric_limits<std::streamsize>::max());
            std::streamsize length = file.gcount();

            // seek back to start of stream
            file.clear();
            file.seekg(0, std::ios_base::beg);

            for (size_t i = 0; i < length / sizeof(candle); i++)
            {
                std::unique_ptr<candle> candle_data = std::make_unique<candle>();

                file.read((char*)candle_data.get(), sizeof(candle));

                m_candles.push_back(std::move(candle_data));
            }
        }

        void start()
        {
            this->read_binary_input();

            // do normalization
            candle curr_candle_backup, previous_candle;
            for (size_t i = 0; i < m_candles.size(); i++)
            {
                std::unique_ptr<candle>& curr_candle = m_candles.at(i);
                this->clone_candle(curr_candle.get(), &curr_candle_backup);

                if (i)
                    curr_candle->normalize(&previous_candle);

                this->clone_candle(&curr_candle_backup, &previous_candle);
            }

            this->write_binary_out();
        }

        void write_binary_out()
        {
            std::string out_dir = m_out_dir / m_input_file.stem();
            std::ofstream output_stream(out_dir + ".bin", std::ios::binary | std::ios::trunc);

            for (const std::unique_ptr<candle>& candle_struct : m_candles)
                output_stream.write((char*)candle_struct.get(), sizeof(candle));

            output_stream.close();
        }
    };
}
