#include<stdio.h>
#include<pthread.h>
#define MAXSIZE 32
#define PRODUCTOR 8
#define CONSUMER 16
//环形队列

//通常作为buffer缓冲区使用

typedef struct ring_buffer {
	int number;
	int read_able;
}ring_buffer;



int front = 0, rear = 0;
int wait_insert_number = 0;
ring_buffer global_buffer[MAXSIZE];
int count = 0;
int sum = 0;


void init_buffer(ring_buffer* ptr) {
	for (int i = 0;i < MAXSIZE;i++) {
		ptr[i].number = i;
		ptr[i].read_able = 0;
	}
}



void check_buffer(ring_buffer* ptr) {
	for (int i = 0;i < MAXSIZE;i++)
		if (ptr[i].number != i || ptr[i].read_able != 0) {
			printf("存在未初始化阶段\n");
			break;
		}
}

//写队列 多生产者
void enqueue_buffer(ring_buffer* ptr) {
	while (1) {
		int tmp_front = front;
		int tmp_rear = rear;
		if(__sync_bool_compare_and_swap(&front, (rear+1)%MAXSIZE, front) == 1){
			continue;
		}
		else{
			if(__sync_bool_compare_and_swap(&rear, tmp_rear, (rear+1)%MAXSIZE) == 1){
				if (__sync_bool_compare_and_swap(&ptr[tmp_rear].read_able, 0, 1) == 1) {
					continue;
				}
			}
			else{
				continue;
			}
		} 
	}
}


//读数据 多消费者
void dequeue_buffer(ring_buffer* ptr) {
	while (1) {
		//一定要保证front的自增性
		int tmp_front = front;
		int tmp_rear = rear;
		if(__sync_bool_compare_and_swap(&front, rear, front) == 1){
			continue;
		}
		else{
			if(__sync_bool_compare_and_swap(&front, tmp_front, (front+1)%MAXSIZE) == 1){
				//确定这个front是唯一不变
				if (__sync_bool_compare_and_swap(&ptr[tmp_front].read_able, 1, 0) == 1) {
					//printf("取出的数据为:%d\n",ptr[tmp_front].number);
					sum+=ptr[tmp_front].number;
					count++;
					if(count == 32){
						if(sum!=496)
							printf("%d the lock free ring_buffer is error!\n",sum);
						//printf("the result is 1+2+3+...+32 = %d\n",sum);
						count = 0;
						sum = 0;
					}
					continue;
				}
			}
			else{
				continue;
			}
		}
	}
}





int main(){
	pthread_t productor[PRODUCTOR], consumer[CONSUMER];
	//初始化
	init_buffer(global_buffer);
	//检查
	check_buffer(global_buffer);
	for (int i = 0;i < PRODUCTOR;i++) {
		pthread_create(&productor[i], NULL,(void*) enqueue_buffer,(void*) global_buffer);
	}

	for (int i = 0;i < CONSUMER;i++) {
		pthread_create(&consumer[i], NULL,(void*) dequeue_buffer,(void*) global_buffer);
	}

	for (int i = 0;i < PRODUCTOR;i++) {
		pthread_join(productor[i], NULL);
	}
	for (int i = 0;i < CONSUMER;i++) {
		pthread_join(consumer[i], NULL);
	}

	return 0;

}
