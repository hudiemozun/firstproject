#include "client.h"

#include <string>
#include <sys/timeb.h>

// 将配置信息以全局的方式定义
// 通过函数读取配置文件进行信息配置
int MAX_CLIENT = 0;
// 客户端链接负载均衡器的端口和IP
int PORT = 0;
char IP[32] = {0};

class Client
{
public:
    // 延时函数
    // 用来区分间隔客户端集群的请求服务
    // 防止一窝蜂发送请求
    void DelayTime()
    {
        int time = MAX_CLIENT;
        timeb delaytime;
        srand(getpid() + ftime(&delaytime));

        usleep(rand()% (time * time * 1000000) + 1);
    }
    // 链接分配的服务器
	void LinkServer(int cliCount)
	{
        uint16_t linkport = _port;
        char *ip = _ip;

		int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		assert(sockfd != -1);

		struct sockaddr_in saddr;
		memset(&saddr, 0, sizeof(saddr));
		saddr.sin_family = AF_INET;
		saddr.sin_port = htons(linkport);
		saddr.sin_addr.s_addr = inet_addr(ip);

		int res = connect(sockfd, (struct sockaddr *)&saddr, sizeof(saddr));
	    assert(res != -1);

		char recvbuff[128] = { 0 };
		int len = sizeof(saddr);

        char curClient[1] = {0};
        curClient[0] = cliCount + '0';

		sendto(sockfd, curClient, 1, 0, (struct sockaddr *)&saddr, sizeof(saddr));
		recvfrom(sockfd, recvbuff, 128, 0, (struct sockaddr *)&saddr, (socklen_t *)&len);

        // 打印服务器返回请求的信息内容
        cout << endl;
        cout << "[client " << cliCount << "]";
	    cout << "current time: " << recvbuff << endl;
        
		close(sockfd);
	}
    // 链接负载均衡器
	void LinkLoadBalancer(int port, char *ip, int cliCount)
	{
        while (1)
        {
            DelayTime();

	        int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	        assert(sockfd != -1);

            uint16_t linkport = port;

		    struct sockaddr_in saddr;
		    memset(&saddr, 0, sizeof(saddr));
	        saddr.sin_family = AF_INET;
	        saddr.sin_port = htons(linkport);
	        saddr.sin_addr.s_addr = inet_addr(ip);

	        int res = connect(sockfd, (struct sockaddr *)&saddr, sizeof(saddr));
	        assert(res != -1);

            char curClient[1] = {0};
            curClient[0] = cliCount + '0';

			char recvbuff[128] = { 0 };
			int len = sizeof(saddr);

			sendto(sockfd, curClient, 1, 0, (struct sockaddr *)&saddr, sizeof(saddr));
			recvfrom(sockfd, recvbuff, 128, 0, (struct sockaddr *)&saddr, (socklen_t *)&len);

            // 解析负载均衡器分配的服务器的IP和端口
            char port[16] = {0};
            strncpy(port, recvbuff, 4);
            _port = atoi(port);

            int j = 0;
            for ( ; recvbuff[j] != '\0'; ++j)
            {
                _ip[j] = recvbuff[j + 4];
            }

            // 客户端与负载均衡器断开链接，
            // 准备和负载均衡器分配的服务器链接获取请求信息
            close(sockfd);
            LinkServer(cliCount);
        }
	}

    // 创建多进程，实现多台客户端
    void MultiProcess()
    {
        pid_t pid;

        // 指定链接负载均衡器的IP和端口
        int port = PORT;
        char ip[32] = {0};
        strcpy(ip, IP);

        int cliCount = 0;
        int i = 0;
        for ( ; i < MAX_CLIENT; ++i )
        {
            pid = fork();
            // 给客户端标明唯一ID，从 1 开始，逐个递增
            cliCount++;

            if ( pid < 0 )
            {
                cout << "client fork error!!" << endl;
            }
            else if ( pid == 0 )
            {
                LinkLoadBalancer(port, ip, cliCount);
                exit(0);
            }
        }
        // 打印创建客户端集群的父进程的信息
        cout << "[client parent]: pid = " << getpid() << endl;
        cout << endl;
    }
private:
	int _port;  // 负载均衡器分配链接服务器的端口
    char _ip[64];  // 负载均衡器分配链接服务器的IP
};

// 读取配置文件信息进行IP和端口配置
void ReadConfigure()
{
    FILE *readfd = fopen("client_myxml.txt", "rb");
    assert( readfd != NULL );

    char temp[32] = {0};
    int i = 0;
    int j = 0;

    fgets(temp, 32, readfd);
    temp[strlen(temp) - 1] = '\0';
    while ( temp[i] != '\0' )
    {
        if ( temp[i] == '=' )
        {
            i++;
            while ( temp[i] != '\0' )
            {
                MAX_CLIENT = temp[i++] - '0';
            }
            break;
        }
        i++;
    }

    i = 0;
    j = 0;
    fgets(temp, 32, readfd);
    temp[strlen(temp) - 1] = '\0';
    while ( temp[i] != '\0' )
    {
        if ( temp[i] == '=' )
        {
            i++;
            char port[32] = {0};
            while ( temp[i] != '\0' )
            {
                port[j++] = temp[i++];
            }
            PORT = atoi(port);
            break;
        }
        i++;
    }

    i = 0;
    j = 0;
    fgets(temp, 32, readfd);
    temp[strlen(temp) - 1] = '\0';
    while ( temp[i] != '\0' )
    {
        if ( temp[i] == '=' )
        {
            i++;
            while ( temp[i] != '\0' )
            {
                IP[j++] = temp[i++];
            }
            break;
        }
        i++;
    }

    fclose(readfd);
}

int main()
{
    ReadConfigure();

    Client clientRun;
    clientRun.MultiProcess();


    exit(0);
}
