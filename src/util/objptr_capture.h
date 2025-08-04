
#ifndef __MESHABLE_H__
#define __MESHABLE_H__

#include <vector>


namespace CGL {

    template<typename T>
    class ObjPtrCapture {
    public:
        ObjPtrCapture() : cur_ptr(nullptr) {};
        ~ObjPtrCapture() {
            release();
        }
        const T *capture() {
            if (cur_ptr == nullptr) {
                create_ptr(&cur_ptr);
            }
            update_cur_ptr(&cur_ptr);
            return cur_ptr;
        }
        const T *current() {
            return cur_ptr;
        }
        void release() {
            if (cur_ptr != nullptr) {
                release_ptr(cur_ptr);
                cur_ptr = nullptr;
            }
        }
    protected:
        virtual void create_ptr(T **) const = 0;
        virtual void update_cur_ptr(T **) const = 0;
        virtual void release_ptr(T *) const = 0;

    private:
        T *cur_ptr;
    };

}


#endif //__MESHABLE_H__