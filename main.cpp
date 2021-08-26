#include <iostream>
#include <fstream>
#include <cmath>

//функция, преобразующая число a в строку из bytes символов в порядке LE (a >= 0)
std::string convert(int a, int bytes)
{
    std::string s;
    if (a < 256)
    {
        unsigned char c = a;
        s += c;
        c = 0;
        for (int i = 0; i < bytes - 1; ++i) s += c;
    }
    else if (a < 65536)
    {
        unsigned char c = a%256;
        s += c;
        c = a >> 8;
        s += c;
        c = 0;
        for (int i = 0; i < bytes - 2; ++i) s += c;
    }
    else if (a < 16777216)
    {
        unsigned char c = a%256;
        s += c;
        c = (a%65536)>>8;
        s += c;
        c = a>>16;
        s += c;
        c = 0;
        for (int i = 0; i < bytes - 3; ++i) s += c;
    }
    return s;
}

int main(int argc, char * argv[])
{
    if (argc != 3)//проверка на точное кол-во аргументов командной строки
    {
        std::cout << "Invalid number of the command line arguments." << std::endl;
        return -1;
    }

    //передача из командной строки значений частоты, амплитуды, длительности и названия файла
    int sizeOfSide = atoi(argv[1]);//длина ширины и высоты в пикселях
    std::string fileName = argv[2];//имя файла вывода
    if(sizeOfSide%8 != 0)//размер ширины и высоты должен быть кратным 8
    {
        std::cout << "The size of side must be n*8 number." << std::endl;
        return -2;
    }

    //вычисление значений полей заголовка
    int bfOffBits = 32;//положение пиксельных данных в файле
    int bfSize = bfOffBits + sizeOfSide * sizeOfSide / 8;//размер файла
    int bcBitCount = 1;//количество бит на пиксель (1 бит поддерживает максимум 2 цвета)

    //запись заголовка
    std::ofstream fout;
    fout.open(fileName, std::ios_base::binary);
    fout << "BM";
    fout << convert(bfSize, 4);
    fout << convert(0, 4);
    fout << convert(bfOffBits, 4);
    fout << convert(12, 4);
    fout << convert(sizeOfSide, 2);
    fout << convert(sizeOfSide, 2);
    fout << convert(1, 2);
    fout << convert(bcBitCount, 2);

    //запись первого цвета в формате RGB
    fout << convert(19, 1);//B
    fout << convert(60, 1);//G
    fout << convert(139, 1);//R

    //запись второго цвета в формате RGB
    fout << convert(179, 1);//B
    fout << convert(222, 1);//G
    fout << convert(245, 1);//R

    //запись пиксельных данных
    unsigned char c;
    if (sizeOfSide%64 == 0)//упрощенная побайтовая запись (только для сторон кратных 64)
    {
        for (int i = 0; i < 8; ++i)//запись 8 рядов клеток (i - индекс клетки по вертикали)
        {
           for (int j = 0; j < sizeOfSide/8; ++j)//запись одного рядя клеток (j - индекс пикселя клетки по вертикали)
           {
               for (int k = 0; k < 8; ++k)//запись горизонтальной линии высотой 1 пиксель (k - индекс клетки по горизонтали)
               {
                   //определение цвета
                   if ((i + k)%2 == 0) c = 0;//индекс первого цвета (побитовая запись - 00000000)
                   else c = 255;//индекс второго цвета (побитовая запись - 11111111)
                   for (int l = 0; l < sizeOfSide/64; ++l)//запись горизонтальной линии шириной 1 клетки
                   {
                       fout << c;
                   }
               }
           }
        }
    }
    else//побитовая запись
    {
        int addSize = 0;//дополнительный размер файла
        int bitsCounter=0;//счетчик битов
        bool bit;//определяет номер бита (цвет пикселя)
        for (int i = 0; i < 8; ++i)
        {
           for (int j = 0; j < sizeOfSide/8; ++j)
           {
               unsigned char c = 0;
               for (int k = 0; k < 8; ++k)
               {
                   if ((i + k)%2 == 0) bit = 0;
                   else bit = 1;
                   for (int l = 0; l < sizeOfSide/8; ++l)
                   {
                       if (bit)
                       {
                           c += pow(2, 7 - bitsCounter%8);//определение позиции бита в байте
                       }
                       if (bitsCounter%8 == 7)//запись байта и последующее обнуление
                       {
                           fout << c;
                           c = 0;
                       }
                       ++bitsCounter;
                   }
               }
               if (sizeOfSide%32 != 0)//добавление нулевых байтов до размера ряда кратного 32
               {
                   char zero = 0;
                   for (int m = 0; m < (32 - sizeOfSide%32)/8; ++m)
                   {
                       fout << zero;
                       ++addSize;//увеличение дополнительного размера файла
                   }
               }
           }
        }
        if (sizeOfSide%32 != 0)//обновление размера файла
        {
            fout.seekp(2, std::ios::beg);
            unsigned char c = bfSize + addSize;
            fout << c;
        }
    }
    fout.close();
    std::cout << "The bmp file has been created." << std::endl;
    return 0;
}
