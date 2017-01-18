#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/time.h>
#include<math.h>
#include "my402list.h"
#include"cs402.h"
#include<sys/types.h>
#include<ctype.h>
#include<signal.h>
#include<sys/stat.h>
typedef struct pack {
	int p_id;
	double interarrivaltime;
	double enterq1, leaveq1;
	double enterq2, leaveq2;
	double enters1, leaves1;
	double enters2, leaves2;
	int tokensreq;
	double arrivaltime;
	int servicetime;
} packet;
typedef struct tfile {
	int l;
	int m;
	int tpacket;
} value;
pthread_t t_ids[4];
int tb_size = 10, tokenno = 0, pt = 3;
float lambda = 1, tokenrate = 1.5;
My402List queue1, queue2;
My402List filedata;
double endtime;
double prev_time, starttv;
int actualtokenno = 0;
int pnum = 20, pcreated = 0;
pthread_mutex_t mutex;
pthread_cond_t cond;
float mu = .35;
struct timeval starttv1;
int packetthread = 0, tokenthread = 0;
int filefound = 0;
char *filename;
struct stat s;
int servicedpackets = 0;
double tokendropped = 0, packetdropped = 0;
int s1count = 0, s2count = 0, q1count, q2count = 0;
double q1total = 0, q2total = 0, s1total = 0, s2total = 0, totalservicethreads =
		0, totalserviceprogram = 0,totalsquare =0;
double totaliatime = 0;
void getparams(int argc, char *argv[]) {
	//fprintf(stdout, "I am in getparams\n");
	FILE *fp;
	int i;
	char buf[1000];
	char delimiter[] = " \t";
	char *t;
	if ((strcmp(argv[0], "warmup2")) != 0 && (strcmp(argv[0], "./warmup2")) != 0)
	{
		fprintf(stdout,
				"Invalid cmd line. Usage: warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]");
		exit(1);
	}
	if(argc%2 == 0)
	{
		fprintf(stderr,"\n Arguments invalid ");
		exit(1);
	}
	for (i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-t") == 0) {
			filefound = 1;
			filename = argv[i + 1];
			fp = fopen(argv[i + 1], "r");
			if (fp == NULL) {
				fprintf(stderr, "Unable to open the trace file\n");
				exit(1);
			}
		}
	}
	if (filefound == 1) {

		stat(filename, &s);
		if (S_ISDIR(s.st_mode)) {
			fprintf(stderr, "Error: %s is a directory \n", filename);
			exit(1);
		}

		for (i = 0; i < argc; i++) {
			if (strcmp(argv[i], "-B") == 0) {
				char *str = argv[i + 1];
				char *err = str;
				strtof(str, &err);
				if (*err != 0)
					fprintf(stderr, "Invalid arguments\n");
				tb_size = atoi(argv[i + 1]);
				if (tb_size < 0 || tb_size > 2147483647) {
					fprintf(stderr, "bucket size is invalid\n");
					exit(0);
				}
			} else if (strcmp(argv[i], "-r") == 0) {
				char *str = argv[i + 1];
				char *err = str;
				strtof(str, &err);
				if (*err != 0)
					fprintf(stderr, "Invalid arguments\n");
				tokenrate = atof(argv[i + 1]);
				if (tokenrate < 0 || tokenrate > 2147483647) {
					fprintf(stderr, "token rate is invalid\n");
					exit(0);
				}
			}
		}

		fgets(buf, sizeof(buf), fp);
		pnum = atoi(buf);
		if (pnum <= 0 || pnum > 2147483647) {
			fprintf(stderr, "packet number is an invalid value\n");
			exit(1);
		}
		int count = 0;
		//printf("number to arrive=%d", pnum);
		while (fgets(buf, sizeof(buf), fp) != NULL) {
			count++;
			value *v = (value *) malloc(sizeof(value));
			if (v == NULL) {
				fprintf(stderr, "Unable to allocate memory\n");
				exit(1);
			}
			t = strtok(buf, delimiter);
			if (t == NULL) {
				fprintf(stderr, "Invalid trace file\n");
				exit(1);
			}
			else if ((atof(t)) < 0) {
				fprintf(stderr, "Lambda cannot have a negative value\n");
				exit(1);
			} else if ((atof(t)) > 10000.0) {
				v->l = 10000;
			}
			else
				v->l = atof(t);
			t++;
			if (isspace(*t) || strcmp(t, "\t"))
				t++;
			t = strtok(NULL, delimiter);
			if (t == NULL) {
				fprintf(stderr, "Invalid trace file\n");
				exit(1);
			}
			if ((atoi(t)) < 0 || (atoi(t)) > 2147483647) {
				fprintf(stderr, "no. of token /packet value is invalid\n");
				exit(1);
			}
			v->tpacket = atoi(t);
			t++;
			if (isspace(*t) || strcmp(t, "\t"))
				t++;
			t = strtok(NULL, "\n");
			if (t == NULL) {
				fprintf(stderr, "Invalid trace file\n");
				exit(1);
			}
			else if ((atoi(t)) < 0) {
				fprintf(stderr, "mu cannot have a negative value\n");
				exit(1);
			} else if ((atoi( t))> 10000) {
				v->m = 10000;

			}
			else
				v->m = atoi(t);
			//fprintf(stdout, "\n %d %d %d", v->l, v->m, v->tpacket);
			My402ListAppend(&filedata, v);

		}
//		My402ListElem *e = My402ListFirst(&filedata);
//		while (e != NULL) {
//			value *v1 = (value *) e->obj;
//			fprintf(stdout, " \n %d %d %d\n", v1->l, v1->m, v1->tpacket);
//			e = My402ListNext(&filedata, e);
//		}

		if (count != pnum) {
			fprintf(stderr, "Error in tfile. Number of lines not equal to num");
			exit(1);
		}

	} else if (filefound == 0) {
		char *str;
		char *err;
		for (i = 0; i < argc; i++) {
			if (strcmp(argv[i], "-lambda") == 0)
			{
				str = argv[i+1];
				err = str;
				strtof(str,&err);
				if(*err != 0)
					fprintf(stderr,"Invalid arguments\n");
				lambda = atof(argv[i + 1]);
				if (lambda < 0) {
					fprintf(stderr, "Lambda cannot have a negative value\n");
					exit(1);
				} else if ((1 / lambda) > 10) {
					lambda = 0.1;

				}
				i++;
			}
			if (strcmp(argv[i], "-mu") == 0) {
				str = argv[i + 1];
				err = str;
				strtof(str, &err);
				if (*err != 0)
					fprintf(stderr, "Invalid arguments\n");
				mu = atof(argv[i + 1]);
				if (mu < 0) {
					fprintf(stderr, "mu cannot have a negative value\n");
					exit(1);
				} else if ((1 / mu) > 10) {
					mu = 0.1;

				}
				i++;
			}
			if (strcmp(argv[i], "-r") == 0) {
				str = argv[i + 1];
				err = str;
				strtof(str, &err);
				if (*err != 0)
					fprintf(stderr,"Invalid arguments\n");
				tokenrate = atof(argv[i + 1]);
				if (tokenrate < 0) {
					fprintf(stderr, "tokenrate cannot have a negative value\n");
					exit(1);
				} else if ((1 / tokenrate) > 10) {
					tokenrate = 0.1;

				}
				i++;
			}
			if (strcmp(argv[i], "-B") == 0) {
				str = argv[i + 1];
				err = str;
				strtof(str, &err);
				if (*err != 0)
					fprintf(stderr, "Invalid arguments\n");
				tb_size = atoi(argv[i + 1]);
				if (tb_size < 0 || tb_size > 2147483647) {
					fprintf(stderr, "bucket size value is invalid\n");
					exit(1);
				}
				i++;
			}
			if (strcmp(argv[i], "-P") == 0) {
				str = argv[i + 1];
				err = str;
				strtof(str, &err);
				if (*err != 0)
					fprintf(stderr,"Invalid arguments\n");
				pt = atoi(argv[i + 1]);
				if (pt < 0 || pt > 2147483647) {
					fprintf(stderr, "no. of token /packet value is invalid\n");
					exit(1);
				}
				i++;
			}
			if (strcmp(argv[i], "-n") == 0) {
				str = argv[i + 1];
				err = str;
				strtof(str, &err);
				if (*err != 0)
					fprintf(stderr,"Invalid arguments\n");
				pnum = atoi(argv[i + 1]);
				if (pnum < 0 || pnum > 2147483647) {
					fprintf(stderr,
							"total number of packet value is invalid\n");
					exit(1);
				}
				i++;
			}
//			} else {
//				fprintf(stderr,
//						"Invalid cmd line option. Usage: warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]");
//				exit(1);
//			}
		}

	}
//	My402ListElem *e = My402ListFirst(&filedata);
//	while (e == NULL) {
//		value *v1 = (value *) e->obj;
//		fprintf(stdout, " \n %d %d %d\n", v1->l, v1->m, v1->tpacket);
//		e = My402ListNext(&filedata, e);
//	}
}
void printdata() {
	fprintf(stdout, "\nEmulation parameters:\n\t");
	fprintf(stdout, "number to arrive = %d\n", pnum);
	if (filefound == 0) {
		fprintf(stdout, "\tlambda = %.3f \n\t", lambda);
		fprintf(stdout, "mu = %.3f \n", mu);	//do point formatting
	}

	fprintf(stdout, "\tr = %.3f \n\t", tokenrate);
	fprintf(stdout, "B = %d \n", tb_size);

	if (filefound == 0)
		fprintf(stdout, "\tP = %d \n", pt);
	if (filefound == 1)
		fprintf(stdout, "\ttsfile = %s \n", filename); //print file name
//	My402ListElem *e = My402ListFirst(&filedata);
//	while (e == NULL) {
//		value *v1 = (value *) e->obj;
//		fprintf(stdout, " \n %d %d %d\n", v1->l, v1->m, v1->tpacket);
//		e = My402ListNext(&filedata, e);
//	}
}
void secondsleep(int ss) {
	struct timespec t;
	t.tv_sec = ss / 1000;
	t.tv_nsec = (ss % 1000) * (1000 * 1000);
	nanosleep(&t, NULL);
}

void exithandler(int signo) {
	//fprintf(stdout,"hererere");
	pthread_cancel(t_ids[0]);
	pthread_cancel(t_ids[1]);
	//fprintf(stdout,"hre1");
	struct timeval tv;
	double currenttime;
	My402ListElem *elem = My402ListFirst(&queue1);
	while (elem != NULL) {
		(void) gettimeofday(&tv, NULL);
		currenttime = ((tv.tv_sec * 1000.0) + (tv.tv_usec / 1000.0));
		packet *p = (packet *) elem->obj;
		fprintf(stdout, "\n%012.3fms: p%d removed from Q1",
				(currenttime - starttv), p->p_id);
		My402ListUnlink(&queue1, elem);
		elem = My402ListFirst(&queue1);
	}
	elem = My402ListFirst(&queue2);
	while (elem != NULL) {
		(void) gettimeofday(&tv, NULL);
		currenttime = ((tv.tv_sec * 1000.0) + (tv.tv_usec / 1000.0));
		packet *p = (packet *) elem->obj;
		fprintf(stdout, "\n%012.3fms: p%d removed from Q2\n",
				(currenttime - starttv), p->p_id);
		My402ListUnlink(&queue2, elem);
		elem = My402ListFirst(&queue2);
	}
	fprintf(stdout, "\n");
	packetthread = 1;
	tokenthread = 1;
	pthread_cond_signal(&cond);
}

void* packetThread() {
	int p = 0;
	value *v;
	double exectime =0,execendtime =0 ;
	while (pnum != 0)
	{
		//fprintf(stdout, "enters pthread\n");
		struct timeval tv;
		//fprintf(stdout, "i am here\n");
		if (filefound == 1) {
			//fprintf(stdout, "hiiii....\n");
			My402ListElem *e = My402ListFirst(&filedata);
//			fprintf(stdout, "i have the queue\n");
//			printf("dasdasdasd %d\n", My402ListLength(&filedata));
			v = (value *) e->obj;
//			fprintf(stdout, "I have the val\n");
//			fprintf(stdout, "val %d\n", v->l);
			My402ListUnlink(&filedata, e);
//			if(((double)(v->l)-(execendtime-exectime))>=0.0)
//			{
//				secondsleep(ceil((double)(v->l)-(execendtime-exectime)));
//			}
			secondsleep(v->l);
			(void) gettimeofday(&tv, NULL);
			exectime = ((tv.tv_sec * 1000.0) + (tv.tv_usec / 1000.0));

		}
		else
		{
//			if(((((1.0) / lambda) * 1000)-(execendtime-exectime)) >=0.0)
//				secondsleep(ceil(((1.0) / lambda) * 1000)-(execendtime-exectime));
			secondsleep((1.0)/lambda*1000);

		}
		(void) gettimeofday(&tv, NULL);
		exectime = ((tv.tv_sec * 1000.0) + (tv.tv_usec / 1000.0));
		packet *newElem = (packet *) malloc(sizeof(packet));
		p++;
		if (newElem == NULL) {
			fprintf(stderr, "\nFailed to allocate memory");
			exit(1);
		}
		pthread_mutex_lock(&mutex);
		newElem->p_id = p;
		pnum--;
		pcreated++;
		if (filefound == 1) {
			newElem->tokensreq = v->tpacket;
			newElem->servicetime = v->m;
		} else {
			newElem->tokensreq = pt;
			newElem->servicetime = ((1.0 / mu) * 1000);
		}
		(void) gettimeofday(&tv, NULL);
		double currenttime = ((tv.tv_sec * 1000.0) + (tv.tv_usec / 1000.0));
		newElem->interarrivaltime = currenttime - prev_time;
		prev_time = currenttime;

		totaliatime += newElem->interarrivaltime;
		newElem->arrivaltime = currenttime;
		if (newElem->tokensreq > tb_size) {
			packetdropped++;
			fprintf(stdout,
					"%012.3fms: p%d arrives, needs %d tokens, inter-arrival time = %0.3fms, dropped\n",
					currenttime - starttv, newElem->p_id, newElem->tokensreq,
					newElem->interarrivaltime);
			prev_time = newElem->arrivaltime;
			pthread_mutex_unlock(&mutex);
			continue;
		}
		if (newElem->tokensreq > 1)
			fprintf(stdout,
					"%012.3fms: p%d arrives, needs %d tokens, inter-arrival time = %0.3fm\n",
					currenttime - starttv, newElem->p_id, newElem->tokensreq,
					newElem->interarrivaltime);
		else
			fprintf(stdout,
					"%012.3fms: p%d arrives, needs %d token, inter-arrival time = %0.3fm\n",
					currenttime - starttv, newElem->p_id, newElem->tokensreq,
					newElem->interarrivaltime);
		(void) gettimeofday(&tv, NULL);
		newElem->enterq1 = (tv.tv_sec * 1000.0) + (tv.tv_usec / 1000.0);
		int val = My402ListAppend(&queue1, newElem);
		if (val == FALSE)
			fprintf(stderr, "Appending fault");
		fprintf(stdout, "%012.3fms: p%d enters Q1\n",
				newElem->enterq1 - starttv, newElem->p_id);
		My402ListElem *elem = My402ListFirst(&queue1);
		if (elem != NULL) {
			//printf("hiiiii");
			packet *p = (packet*) elem->obj;
			if (p->tokensreq <= tokenno) {
				(void) gettimeofday(&tv, NULL);
				p->leaveq1 = (tv.tv_sec * 1000.0) + (tv.tv_usec / 1000.0);
				tokenno = tokenno - (p->tokensreq);
				if (tokenno > 1)
					fprintf(stdout,
							"%012.3fms: p%d leaves Q1, time in Q1 = %0.3fms, token bucket now has %d tokens\n",
							p->leaveq1 - starttv, p->p_id,
							(p->leaveq1 - p->enterq1), tokenno);
				else
					fprintf(stdout,
							"%012.3fms: p%d leaves Q1, time in Q1 = %0.3fms, token bucket now has %d token\n",
							p->leaveq1 - starttv, p->p_id,
							(p->leaveq1 - p->enterq1), tokenno);
				My402ListUnlink(&queue1, elem);
				int val1 = My402ListAppend(&queue2, p);
				currenttime = ((tv.tv_sec * 1000.0) + (tv.tv_usec / 1000.0));
				p->enterq2 = currenttime;
				q1count++;
				q1total += (p->leaveq1 - p->enterq1);
				fprintf(stdout, "%012.3fms: p%d enters Q2\n",
						currenttime - starttv, p->p_id);
				if (val1 == FALSE)
					fprintf(stderr, "\nAppending fault");
				pthread_cond_signal(&cond);
			}
		}
		pthread_mutex_unlock(&mutex);
		(void) gettimeofday(&tv, NULL);
		execendtime = ((tv.tv_sec * 1000.0) + (tv.tv_usec / 1000.0));
	}
	pthread_mutex_lock(&mutex);
	packetthread = 1;
	pthread_mutex_unlock(&mutex);
	//printf("Exiting the packet thread\n");
	return (void *) 0;
}

void* tokenThread()
{
	struct timeval tv;
	double currenttime;
	double exectime=0,execendtime=0;
	//printf("i am here in packet thread)");
	while (packetthread == 0 || My402ListEmpty(&queue1) != TRUE)
	{
		//fprintf(stdout, "enters tokenthread\n");
		//int flag = 0;
//		if(((((1.0) / tokenrate) * 1000)-(execendtime-exectime)) >=0.0)
//		{
//			secondsleep(ceil(((1.0) / tokenrate) * 1000)-(execendtime-exectime));
//
//		}
		secondsleep((1.0)/tokenrate*1000);
		(void) gettimeofday(&tv, NULL);
		exectime = ((tv.tv_sec * 1000.0) + (tv.tv_usec / 1000.0));
		pthread_mutex_lock(&mutex);
		actualtokenno++;
		if (tb_size > tokenno)
		{
			tokenno++;
			(void) gettimeofday(&tv, NULL);
			currenttime = (tv.tv_sec * 1000.0) + (tv.tv_usec / 1000.0);
			if (tokenno > 1)
				fprintf(stdout,
						"%012.3fms: token t%d arrives, token bucket now has %d tokens\n",
						currenttime - starttv, actualtokenno, tokenno);
			else
				fprintf(stdout,
						"%012.3fms: token t%d arrives, token bucket now has %d token\n",
						currenttime - starttv, actualtokenno, tokenno);

		} else {
			(void) gettimeofday(&tv, NULL);
			currenttime = (tv.tv_sec * 1000.0) + (tv.tv_usec / 1000.0);
			fprintf(stdout, "%012.3fms: token t%d arrives, dropped\n",
					currenttime - starttv, actualtokenno);
			tokendropped++;
		}
		My402ListElem *elem;
		for (elem = My402ListFirst(&queue1); elem != NULL; elem =
				My402ListFirst(&queue1)) {
			//printf("enter hiiii");
			packet *p = (packet *) elem->obj;
			if (p->tokensreq <= tokenno) {
				(void) gettimeofday(&tv, NULL);
				p->leaveq1 = (tv.tv_sec * 1000.0) + (tv.tv_usec / 1000.0);
				if (tokenno > 1)
					fprintf(stdout,
							"%012.3fms: p%d leaves Q1, time in Q1 = %0.3fms, token bucket now has %d tokens\n",
							p->leaveq1 - starttv, p->p_id,
							(p->leaveq1 - p->enterq1), tokenno);
				else
					fprintf(stdout,
							"%012.3fms: p%d leaves Q1, time in Q1 = %0.3fms, token bucket now has %d token\n",
							p->leaveq1 - starttv, p->p_id,
							(p->leaveq1 - p->enterq1), tokenno);
				My402ListUnlink(&queue1, elem);
				int val1 = My402ListAppend(&queue2, p);
				currenttime = ((tv.tv_sec * 1000.0) + (tv.tv_usec / 1000.0));
				p->enterq2 = currenttime;
				q1count++;
				q1total += (p->leaveq1 - p->enterq1);
				fprintf(stdout, "%012.3fms: p%d enters Q2\n",
						currenttime - starttv, p->p_id);
				if (val1 == FALSE)
					fprintf(stderr, "Appending fault");
				pthread_cond_signal(&cond);
				tokenno = tokenno - (p->tokensreq);
			} else {
				pthread_mutex_unlock(&mutex);
				break;
			}
		}
		pthread_mutex_unlock(&mutex);(void) gettimeofday(&tv, NULL);
		execendtime = ((tv.tv_sec * 1000.0) + (tv.tv_usec / 1000.0));
	}
	pthread_mutex_lock(&mutex);
	tokenthread = 1;
	pthread_mutex_unlock(&mutex);
	pthread_cond_signal(&cond);
	//printf("Exiting the token thread\n");
	return (void *) 0;
}

void* server1Thread() {
	struct timeval tv;
	double currenttime;
	while (My402ListEmpty(&queue2) == FALSE || tokenthread == 0) {
		//fprintf(stdout, "enters server1thread\n");
		pthread_mutex_lock(&mutex);
		while (My402ListEmpty(&queue2) && tokenthread == 0) {
			pthread_cond_wait(&cond, &mutex);
		}
		if (My402ListEmpty(&queue2) && tokenthread == 1) {
			pthread_mutex_unlock(&mutex);
			break;
		}
		My402ListElem *elem = My402ListFirst(&queue2);
		packet *p = (packet*) elem->obj;
		(void) gettimeofday(&tv, NULL);
		currenttime = (tv.tv_sec * 1000.0) + (tv.tv_usec / 1000.0);
		p->leaveq2 = currenttime;
		My402ListUnlink(&queue2, elem);
		fprintf(stdout, "%012.3fms: p%d leaves Q2, time in Q2 = %05.3fms\n",
				(currenttime - starttv), p->p_id, (p->leaveq2 - p->enterq2));
		(void) gettimeofday(&tv, NULL);
		currenttime = (tv.tv_sec * 1000.0) + (tv.tv_usec / 1000.0);
		p->enters1 = currenttime;
		q2count++;
		q2total += (p->leaveq2 - p->enterq2);
		fprintf(stdout, "%012.3fms: p%d begins service at S1, requesting %d",
				(currenttime - starttv), p->p_id, p->servicetime);
		fprintf(stdout, "ms of service\n");
		pthread_mutex_unlock(&mutex);
		secondsleep((p->servicetime));
		pthread_mutex_lock(&mutex);
		(void) gettimeofday(&tv, NULL);
		currenttime = (tv.tv_sec * 1000.0) + (tv.tv_usec / 1000.0);
		p->leaves1 = currenttime;
		s1count++;
		servicedpackets++;
		pthread_mutex_unlock(&mutex);
		s1total += (p->leaves1 - p->enters1);
		totalservicethreads += (p->leaves1 - p->enters1);
		totalserviceprogram += (p->leaves1 - p->arrivaltime);
		totalsquare += ((p->leaves1 - p->arrivaltime) * (p->leaves1 - p->arrivaltime));
		fprintf(stdout,
				"%012.3fms: p%d departs from S1, service time = %08.3fms, time in system = %08.3fms\n",
				currenttime - starttv, p->p_id, (p->leaves1 - p->enters1),
				currenttime - p->arrivaltime);
	}
	pthread_cond_signal(&cond);
	//printf("Exiting the server thread 1\n");
	return (void *) 0;
}

void* server2Thread() {
	struct timeval tv;
//fprintf(stdout, "enters server2thread\n");
	double currenttime;
	while (My402ListEmpty(&queue2) == FALSE || tokenthread == 0) {
		pthread_mutex_lock(&mutex);
		while (My402ListEmpty(&queue2) && tokenthread == 0) {
			pthread_cond_wait(&cond, &mutex);
		}
		if (My402ListEmpty(&queue2) && tokenthread == 1) {
			pthread_mutex_unlock(&mutex);
			break;
		}
		My402ListElem *elem = My402ListFirst(&queue2);
		packet *p = (packet *) elem->obj;
		(void) gettimeofday(&tv, NULL);
		currenttime = (tv.tv_sec * 1000.0) + (tv.tv_usec / 1000.0);
		p->leaveq2 = currenttime;
		My402ListUnlink(&queue2, elem);
		fprintf(stdout, "%012.3fms: p%d leaves Q2, time in Q2 = %05.3fms\n",
				currenttime - starttv, p->p_id, (p->leaveq2 - p->enterq2));
		(void) gettimeofday(&tv, NULL);
		currenttime = (tv.tv_sec * 1000.0) + (tv.tv_usec / 1000.0);
		p->enters2 = currenttime;
		q2count++;
		q2total += (p->leaveq2 - p->enterq2);
		fprintf(stdout, "%012.3fms: p%d begins service at S2, requesting %d",
				currenttime - starttv, p->p_id, p->servicetime);
		fprintf(stdout, "ms of service\n");
		pthread_mutex_unlock(&mutex);
		secondsleep((p->servicetime));
		pthread_mutex_lock(&mutex);
		(void) gettimeofday(&tv, NULL);
		currenttime = (tv.tv_sec * 1000.0) + (tv.tv_usec / 1000.0);
		p->leaves2 = currenttime;
		s2count++;
		servicedpackets++;
		pthread_mutex_unlock(&mutex);
		s2total +=  (p->leaves2 - p->enters2);
		totalservicethreads +=  (p->leaves2 - p->enters2);
		totalserviceprogram += (p->leaves2 - p->arrivaltime);
		totalsquare += ((p->leaves2 - p->arrivaltime) * (p->leaves2 - p->arrivaltime));
		fprintf(stdout,
				"%012.3fms: p%d departs from S2, service time = %08.3fms, time in system = %08.3fms\n",
				currenttime - starttv, p->p_id, (p->leaves2 - p->enters2),
				currenttime - p->arrivaltime);
	}
	pthread_cond_signal(&cond);
	//printf("Exiting the server thread 2\n");
	return (void *) 0;
}
void printstats() {
	fprintf(stdout, "\nStatistics:\n\n\t");
	if(pcreated != 0)
		printf("average packet inter-arrival time = %.6g\n\t",
			(totaliatime / (pcreated*1000)));
	else
		printf("average packet inter-arrival time = n/a\n\t");
	if(servicedpackets != 0)
		printf("average packet service time = %.6g\n\n\t",
				(totalservicethreads / (servicedpackets*1000)));
	else
		printf("average packet service time = n/a\n\n\t");
	printf("average number of packets in Q1 = %.6g\n\t", (q1total / (endtime-starttv)));
	printf("average number of packets in Q2 = %.6g\n\t", (q2total / (endtime-starttv)));
	printf("average number of packets at S1 = %.6g\n\t", (s1total / (endtime-starttv)));
	printf("average number of packets at S2 = %.6g\n\n\t", (s2total / (endtime-starttv)));
	double sd =0;
	if(servicedpackets != 0)
	{
		sd = (totalsquare/servicedpackets)-((totalserviceprogram/servicedpackets)*(totalserviceprogram/servicedpackets));
		printf("average time a packet spent in system = %.6g\n\t",
				(totalserviceprogram / (servicedpackets*1000)));
		printf("standard deviation for time spent in system = %.6g\n\n\t",((sqrt(sd))/1000));
	}
	else
	{
		printf("average time a packet spent in system = n/a\n\t");
		printf("standard deviation for time spent in system = n/a\n\n\t");
	}
	if(actualtokenno !=0)
		printf("token drop probability = %.6g\n\t",(tokendropped/actualtokenno));
	else
		printf("token drop probability = n/a\n\t");
	if(pcreated != 0)
		printf("packet drop probability = %.6g\n",(packetdropped/pcreated));
	else
		printf("packet drop probability = n/a\n");

}

int main(int argc, char *argv[]) {
	int err[4];
	int i = 0;
	int error = 0;
	struct timeval tv;
	memset(&filedata, 0, sizeof(My402List));
	getparams(argc, argv);
	(void) gettimeofday(&starttv1, NULL);
	memset(&queue1, 0, sizeof(My402List));
	memset(&queue2, 0, sizeof(My402List));
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cond, NULL);
	prev_time = (starttv1.tv_sec * 1000.0) + (starttv1.tv_usec / 1000.0);
	starttv = prev_time;
	fprintf(stdout, "\n%012.3fms: emulation begins", starttv - starttv);
	printdata();
	err[0] = pthread_create(&(t_ids[0]), NULL, &packetThread, NULL);
	err[1] = pthread_create(&(t_ids[1]), NULL, &tokenThread, NULL);
	err[2] = pthread_create(&(t_ids[2]), NULL, &server1Thread, NULL);
	err[3] = pthread_create(&(t_ids[3]), NULL, &server2Thread, NULL);
	for (i = 0; i < 4; i++) {
		if (err[i] != 0) {
			error = err[i];
			fprintf(stderr, "\n Can't create a thread : [%s]", strerror(error));

		}
	}
	signal(SIGINT, exithandler);
	(void) pthread_join(t_ids[0], NULL);
	(void) pthread_join(t_ids[1], NULL);
	(void) pthread_join(t_ids[2], NULL);
	(void) pthread_join(t_ids[3], NULL);
	(void) gettimeofday(&tv, NULL);
	endtime = (tv.tv_sec * 1000.0) + (tv.tv_usec / 1000.0);
	fprintf(stdout, "%012.3fms: emulation ends\n", endtime - starttv);
	printstats();
	return 0;
}

