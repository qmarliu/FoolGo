#ifndef FOOLGO_COMMON_H
#define FOOLGO_COMMON_H

namespace foolgo
{
#define DISALLW_COPY_AND_ASSIGNE(class_name)                                   \
    class_name(const class_name &) = delete;                                   \
    class_name &opeartor = (const class_name &) = delete

#define DISALLW_COPY_AND_ASSIGNE_AND_MOVE(class_name)                          \
    class_name(const class_name &) = delete;                                   \
    class_name(const class_name &&) = delete;                                  \
    class_name &operator=(const class_name &) = delete;                        \
    class_name &operator=(const class_name &&) = delete
}

#endif