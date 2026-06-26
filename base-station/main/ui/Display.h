#ifndef Display_h
#define Display_h

class Display
{
    public:
        void start();

        inline lv_obj_t *screen()
        {
            return _parent;
        }

    private:
        lv_obj_t *_parent;
};

#endif