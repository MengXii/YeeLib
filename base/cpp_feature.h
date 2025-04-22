#define ALWAYS_INLINE inline __attribute__((always_inline))

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName &) = delete; \
    TypeName & operator= (const TypeName &) = delete;
