#ifndef MOMO_COMMON_CLASS_UTILS
#define MOMO_COMMON_CLASS_UTILS



#define MOMO_SETUP_COPY( ClassName, Action )                                                     \
    ClassName( const ClassName& ) = Action;                                                        \
    ClassName& operator=( const ClassName& ) = Action

#define MOMO_SETUP_MOVE( ClassName, Action )                                                     \
    ClassName( ClassName&& ) = Action;                                                             \
    ClassName& operator=( ClassName&& ) = Action

#define MOMO_MAKE_NONCOPYABLE( ClassName ) MOMO_SETUP_COPY( ClassName, delete )
#define MOMO_MAKE_COPYABLE( ClassName ) MOMO_SETUP_COPY( ClassName, default )
#define MOMO_MAKE_NONMOVABLE( ClassName ) MOMO_SETUP_MOVE( ClassName, delete )
#define MOMO_MAKE_MOVABLE( ClassName ) MOMO_SETUP_MOVE( ClassName, default )





#endif  //MOMO_COMMON_CLASS_UTILS

