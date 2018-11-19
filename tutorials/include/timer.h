#pragma once

#include <iostream>
#include <chrono>
#include <string>


class Timer
{
public:
    typedef std::chrono::high_resolution_clock Clock;
    Timer()
    {

    };

    void start(int line, std::string comment = "")
    {
        m_t1 = Clock::now();
        m_startLine = line;
        m_comment = comment;
    }

    void stop(int line)
    {
        m_t2 = Clock::now();
        m_stopLine = line;
    }

    uint nanoseconds()
    {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(m_t2 - m_t1).count();
    }

    void display()
    {
        std::cout <<  m_startLine << "-" << m_stopLine << " " << m_comment << ": "
              << nanoseconds()
              << " nanoseconds" << std::endl;
    }

    void end(int line)
    {
        stop(line);
        display();

    }

private:
    std::chrono::system_clock::time_point m_t1;
    std::chrono::system_clock::time_point m_t2;
    int m_startLine {-1};
    int m_stopLine {-1};
    std::string m_comment;

};

#define START start(__LINE__)
#define START_C(comment) start(__LINE__, comment)
#define END end(__LINE__)