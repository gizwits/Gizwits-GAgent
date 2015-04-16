#ifndef _HAL_RECEIVE_H_
#define _HAL_RECEIVE_H_

#define   HAL_BUF_SIZE 1024
int32 get_data_len( int32 from, int32 to );
int32 get_available_buf_space(int32 pos_current, int32 pos_start);
int32 advance_pos( int32 pos_current, int32 addition0 );
void move_data_backward( uint8 *buf, int32 p_start_pos, int32 p_end_pos, int32 move_len);
int32 get_last_pack_head_pos( uint8 *buf, int32 p_start, int32 p_check);
int32  hal_ReceivepOnePack();
void hal_ReceiveInit( );

#endif