#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>


struct fork {
	int id;
	pthread_mutex_t mutex;
};


struct philosopher {
	int id, eat_count;
	char algo;
	struct fork *left_fork, *right_fork;
	pthread_t thread;
};


void * philosopher(struct philosopher *ph)
{
	for (int i = 0; i < ph->eat_count; ++i) {
		// Думаем
		int t = random() % 5 + 1;
		printf("Ph %3d: Thinking (%3d) for %3d sec...\n", ph->id, i, t);
		fflush(stdout);
		sleep(t);
		
		// Захватываем левую и правую вилки
		if (ph->algo == 0) {
			pthread_mutex_lock(&ph->left_fork->mutex);
			pthread_mutex_lock(&ph->right_fork->mutex);
		} else {
			pthread_mutex_lock(&ph->left_fork->mutex);
			if (pthread_mutex_trylock(&ph->right_fork->mutex)) {
				pthread_mutex_unlock(&ph->left_fork->mutex);
				printf("Ph %3d: Fork %3d (%3d) is busy!\n", ph->id, ph->right_fork->id, i);
				fflush(stdout);
				--i;
				continue;
			}
		}
		
		// Едим
		t = random() % 5 + 1;
		printf("Ph %3d: Eating   (%3d) for %3d sec...\n", ph->id, i, t);
		fflush(stdout);
		sleep(t);
		
		// Освобождаемм правую и левую вилки
		pthread_mutex_unlock(&ph->right_fork->mutex);
		pthread_mutex_unlock(&ph->left_fork->mutex);
	}
	pthread_exit(NULL);
}


int main(int argc, const char **argv)
{
	if (argc != 4) {
		fprintf(stderr, "Incorrect arguments!\nUsage: %s PHILOSOPHERS ALGO CYCLES\n", argv[0]);
		return 1;
	}
	
	// Получение параметров
	int philosophers_count = 0, algo = 0, eat_count = 0;
	sscanf(argv[1], "%d", &philosophers_count);
	sscanf(argv[2], "%d", &algo);
	sscanf(argv[3], "%d", &eat_count);
	
	if (algo) algo = 1;
	
	srandomdev();
	
	// Создаём вилки
	struct fork forks[philosophers_count];
	for (int i = 0; i < philosophers_count; ++i) {
		forks[i].id = i;
		if (pthread_mutex_init(&forks[i].mutex, NULL)) {
			fprintf(stderr, "Error: Can't create fork!\n");
			for (int j = 0; j < i; ++j)
				pthread_mutex_destroy(&forks[j].mutex);
			return 2;
		}
	}
	
	// Создаём философов
	struct philosopher philosophers[philosophers_count];
	for (int i = 0; i < philosophers_count; ++i) {
		philosophers[i].id = i;
		philosophers[i].eat_count = eat_count;
		philosophers[i].algo = (char)algo;
		philosophers[i].left_fork = forks + i;
		philosophers[i].right_fork = forks + i + 1;
	}
	philosophers[philosophers_count - 1].right_fork = forks + 0;
	
	// Запускаем философов
	for (int i = 0; i < philosophers_count; ++i)
		if (pthread_create(&philosophers[i].thread, NULL, (void * (*)(void *))philosopher, philosophers + i) != 0) {
			// Убиваем философов
			for (int j = 0; j < i; ++j)
				pthread_kill(philosophers[j].thread, SIGKILL);
			
			// Уничтожаем вилки
			for (int j = 0; j < philosophers_count; ++j)
				pthread_mutex_destroy(&forks[j].mutex);
			
			return 3;
		}
	
	// Ждём философов
	for (int i = 0; i < philosophers_count; ++i) {
		void *res;
		pthread_join(philosophers[i].thread, &res);
	}
	
	// Уничтожаем вилки
	for (int i = 0; i < philosophers_count; ++i)
		pthread_mutex_destroy(&forks[i].mutex);
	return 0;
}