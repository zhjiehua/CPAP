#include "cmd_queue.h"
#include "uart.h"

#ifdef __cplusplus
extern "C" {
#endif

QUEUE lcd_que = {0,0,0};  //指令队列
QUEUE user_que = {0,0,0};

void queue_reset(QUEUE *que)
{
	que->_head = que->_tail = 0;
	
	que->_cmd_pos = 0;
}

//void queue_push(QUEUE *que, qdata _data)
//{
//	qsize pos = (que->_head+1)%QUEUE_MAX_SIZE;
//	if(pos!=que->_tail)//非满状态
//	{
//		que->_data[que->_head] = _data;
//		que->_head = pos;
//	}
//}

//从队列中取一个数据
void queue_pop(QUEUE *que, qdata* _data)
{
	if(que->_tail!=que->_head)//非空状态
	{
		*_data = que->_data[que->_tail];
		que->_tail = (que->_tail+1)%QUEUE_MAX_SIZE;
	}
}

//获取队列中有效数据个数
static qsize queue_size(QUEUE *que)
{
	return ((que->_head+QUEUE_MAX_SIZE-que->_tail)%QUEUE_MAX_SIZE);
}

qsize queue_find_cmd(QUEUE *que, qdata *buffer, qsize buf_len)
{
    if(que == &user_que)//用户接口命令提取
    {
        static uint32_t cmd_state = 0;
        qsize cmd_size = 0;
    	qdata _data = 0;

    	while(queue_size(que)>0)
    	{
    		//取一个数据
    		queue_pop(que, &_data);
    
    		if(que->_cmd_pos==0&&_data!=USER_CMD_HEAD)//指令第一字节必须是帧头，否则跳过
    		    continue;
    
    		if(que->_cmd_pos<buf_len)//防止缓冲区溢出
    			buffer[que->_cmd_pos++] = _data;
    
    		cmd_state = ((cmd_state<<8)|_data);//拼接最后4个字节，组成一个32位整数
    
    		//最后4个字节与帧尾匹配，得到完整帧
    		if(cmd_state==USER_CMD_TAIL)
    		{
    			cmd_size = que->_cmd_pos; //指令字节长度
    			cmd_state = 0;  //重新检测帧尾
    			que->_cmd_pos = 0; //复位指令指针
    
    			return cmd_size;
    		}
    	}
    }
    else
    {
        uint32_t cmdSize = 0;
    	qdata _data;
    	static uint16_t cmdHead = 0;
    	static uint8_t cmdLen = 3;
    
    	while(queue_size(que)>0)
    	{
    		queue_pop(que, &_data);
    
    		cmdHead = (cmdHead << 8) | _data;
    
    		if(que->_cmd_pos == 0)
    		{
    			if(cmdHead != LCD_CMD_HEAD)//指令第一个字节必须是帧头，否则跳过
    				continue;
    			else
    			{
                    buffer[que->_cmd_pos++] = (LCD_CMD_HEAD>>8);
    			}
    		}

            buffer[que->_cmd_pos++] = _data;

    		if(que->_cmd_pos == 3)
                cmdLen = _data + 3;
    
    		//凑够5字节得到完整帧
    		if (que->_cmd_pos >= cmdLen)
    		{
    			cmdLen = 3;
    			cmdSize = que->_cmd_pos; //指令字节长度
    			que->_cmd_pos = 0; //复位指令指针
    			cmdHead = 0;

    			return cmdSize;
    		} 
    	}     
    }

	return 0;//没有形成完整的一帧   
}

#ifdef __cplusplus
}
#endif
