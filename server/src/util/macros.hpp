#pragma once

#define SERVER_LOG_SYSTEM

#ifdef SERVER_LOG_SYSTEM
#define LOG_INFO(x) std::cout << "\033[1;37m[INFO]:\033[0m " << x << std::endl;
#define LOG_ERR(x)  std::cout << "\033[1;31m[ERR]:\033[0m " << x << std::endl;
#define LOG_TMP(x)  std::cout << "\033[1;35m[TMP]:\033[0m " << x << std::endl;
#else
#define LOG_INFO(x)
#define LOG_ERR(x)
#define LOG_TMP(x)
#endif
