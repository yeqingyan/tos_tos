// Ref. 
// http://workforce.cup.edu/little/serial.html
// http://www.lammertbies.nl/comm/info/RS-232.html
// https://github.com/torvalds/linux/blob/master/drivers/usb/serial/pl2303.c
// TODO more explaination for pl2303
#include <kernel.h>

PORT serial_port;
PORT serial_process_port;
PORT serial_reader_port;

void serial_reader_process(PROCESS self, PARAM param) {
  PROCESS serial_process;
  int index;
  char serial_buf[4];
  while(1) {
    index = 0;
    //receive message form Serial process, this message contains the number of bytes to read in Serial_Message.len_input_buffer
    Serial_Message *msg = (Serial_Message*)receive(&serial_process);
    //read as many bytes requested from COM1 using wait_for_interrupt(COM1_IRQ) and inportb(COM1_PORT)
    if(msg->input_buffer != NULL) {
        while(index != msg->len_input_buffer) {
            RecvFromUSB((char *)&serial_buf, 4);
            *(msg->input_buffer+index) = serial_buf[3];
            kprintf("Got %x", serial_buf[3]);
            index ++;    
        }
        //RecvFromUSB(msg->input_buffer, msg->len_input_buffer);
        // send message to COM process to signal that all bypes have been read
    }
    message(serial_process_port, msg);
  }
}

void serial_process(PROCESS self, PARAM param) {
  PROCESS sender;
  PROCESS serial_reader;
  char *output_ptr;  

  serial_process_port = create_port();
  serial_reader_port = create_process(serial_reader_process, 7, 0, "Serial Reader Process");
  
  while(1){
      // receive message from user process
      Serial_Message *msg = (Serial_Message*)receive(&sender);
      // We can send string here, but since train emulation return byte by byte, we send string
      // byte by byte.  
      output_ptr = msg->output_buffer;
      while(*(output_ptr) != '\0') {
        //while(!(inportb(COM1_PORT+5) & (1<<5)));
        SendToUSB(output_ptr, 1);
        //SendToUSB(output_ptr, k_strlen((const char *)output_ptr));
        output_ptr ++;
      }
      
      message(serial_reader_port, msg);
      // wait for message from COM reader process that signals all bytes have been read
      receive(&serial_reader);
      // reply to user process to signal that all I/O has been completed
      reply(sender);
    }
}

void init_serial()
{
    // init serial_port
    serial_port = create_process(serial_process, 7, 0, "SERIAL Process");
}
