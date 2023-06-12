#define ALWAYS_INLINE inline __attribute__((always_inline))

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
    Typename(const Typename &) = delete; \
    Typename & operator= (const Typename &) = delete;
