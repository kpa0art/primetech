#include <cstdint>
#include <iostream>

const uint32_t MAX_DATA_SIZE  = 1020;                  // 1 kb

class DataBlock {

    uint32_t _number;
    char _data[MAX_DATA_SIZE];

public:
    DataBlock( uint32_t number) : _number(number) {}
};


const uint32_t MAX_BLOCK_SIZE = sizeof(DataBlock(0));

int main(int argc, char *argv[]) {

    std::cout << "Размер блока: " << MAX_BLOCK_SIZE << " байт" << std::endl;
    
    return 0;
}