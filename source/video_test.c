#include <kernel.h>

// 5 x 15
// Row  
// Col 640 Row 400
// Width 40 Height 240
WINDOW lines_wnd = {80, 25, 45, 15, 0, 0, ' '};
WINDOW *lines_ptr = &lines_wnd;

PROCESS lines_proc_ptr;

int last_x=640, last_y=400, cur_x, cur_y;
void lines_stop() {
    remove_ready_queue(lines_proc_ptr);
    clear_window(lines_ptr);
}

void lines_start() {
    add_ready_queue(lines_proc_ptr);
}
void lines_proc(PROCESS self, PARAM param)
{
    unsigned short line_color = 0;
    remove_ready_queue(lines_proc_ptr);
    resign();
    while(1) {
        cur_x = 640 + random() % 360;
        cur_y = 400 + random() % 240;
        draw_line(last_x, last_y, cur_x, cur_y, line_color);
        line_color += 1;
        line_color = line_color % 655355;
        last_x = cur_x;
        last_y = cur_y;
        Wait(1000);
    }          
}

void init_lines_test() {    
    PORT lines_port;
    lines_port = create_process(lines_proc, 5, 0, "Draw Line Test");
    lines_proc_ptr = lines_port->owner;
}