#ifndef _CMD_QUEUE
#define _CMD_QUEUE
#include "hmi_driver.h"

typedef unsigned char qdata;
typedef unsigned short qsize;

//#define QUEUE_MAX_SIZE 20 //128  /*!< 指令接收缓冲区大小，根据需要调整，尽量设置大一些*/

//#define USER_CMD_HEAD 0XEE  //帧头
//#define USER_CMD_TAIL 0XFFFCFFFF //帧尾

typedef struct _QUEUE
{
	qsize _head; //队列头
	qsize _tail;  //队列尾
	qdata _data[QUEUE_MAX_SIZE];	//队列数据缓存区

	qsize _cmd_pos; //当前指令指针位置
}QUEUE;

/*! 
 *  \brief  清空指令数据
 */
void queue_reset(QUEUE *que);

/*! 
 * \brief  添加指令数据
 * \detial 串口接收的数据，通过此函数放入指令队列 
 *  \param  _data 指令数据
 */
void queue_push(QUEUE *que, qdata _data);

/*! 
 *  \brief  从指令队列中取出一条完整的指令
 *  \param  cmd 指令接收缓存区
 *  \param  buf_len 指令接收缓存区大小
 *  \return  指令长度，0表示队列中无完整指令
 */
qsize queue_find_cmd(QUEUE *que, qdata *cmd,qsize buf_len);

void queue_pop(QUEUE *que, qdata* _data);

extern QUEUE lcd_que;
extern QUEUE user_que;

#endif
