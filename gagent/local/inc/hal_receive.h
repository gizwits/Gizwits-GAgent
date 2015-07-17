#ifndef _HAL_RECEIVE_H_
#define _HAL_RECEIVE_H_

#define   HAL_BUF_SIZE 1024
#define   HAL_BUF_MASK  (HAL_BUF_SIZE - 1)
int32 get_available_buf_space(int32 pos_current, int32 pos_start);
void move_data_backward( uint8 *buf, int32 p_start_pos, int32 p_end_pos, int32 move_len);
void hal_ReceiveInit( void );
int32 GAgent_Local_RecAll(pgcontext pgc);
int32 GAgent_Local_WaitDataReady(pgcontext pgc, uint32 timeoutS, uint32 timeoutMs);
int32 GAgent_Local_IsDataValid(pgcontext pgc);

#endif