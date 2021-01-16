#ifndef UTIL_LOG_H_
#define UTIL_LOG_H_

#define LOG(Msgs) std::cout << Msgs << std::endl;
#define LOGn(Msgs)

#define ERROR(Exception, Msgs) \
{ std::stringstream s; s << Msgs; throw std::Exception(s.str()); }

#endif
