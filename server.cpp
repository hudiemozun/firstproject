#include "server.h"

#include <time.h>

// 将配置信息以全局的方式定义
// 通过函数读取配置文件进行信息配置

// 最大服务器创建数量
int MAX_SERVER = 0;
// 服务器配置基准端口和IP
int PORT = 0;
char IP[32] = {0};

class Server
{
public:
	// 获取当前系统时间
    void GetTime()
	{
		time_t temp;
		temp = time(NULL);

        strcpy(_currentTime, ctime(&temp));
	}
	// 处理负载均衡器分配过来客户端的请求
    void LinkClient(uint16_t port, char *ip, int serCount)
	{
		int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		assert(sockfd != -1);

		struct sockaddr_in saddr, caddr;
		memset(&saddr, 0, sizeof(saddr));
		saddr.sin_family = AF_INET;
		saddr.sin_port = htons(port);
		saddr.sin_addr.s_addr = inet_addr(ip);

		int res = bind(sockfd, (struct sockaddr *)&saddr, sizeof(saddr));
		assert(res != -1);

		while (1)
		{
			char recvbuff[128] = { 0 };
			int len = sizeof(caddr);

			recvfrom(sockfd, recvbuff, 127, 0, (struct sockaddr *)&caddr, (socklen_t *)&len);
            
            cout << endl;
            cout << "[server " << serCount << "]: pid = " << getpid() << ",port = " << port << endl;
			cout << "[client " << recvbuff << "]: request time" << endl;

            GetTime();
			sendto(sockfd, _currentTime, strlen(_currentTime), 0, (struct sockaddr *)&caddr, sizeof(caddr));
		}
		close(sockfd);
	}
    // 多进程创建多台服务器
    void MultiProcess()
    {
        pid_t pid;
        int serCount = 0;

        // 这里设定给每台服务器绑定端口和IP的基准值
        uint16_t port = PORT;
        char ip[32] = {0};
        strcpy(ip, IP);

        int i = 0;
        for ( ; i < MAX_SERVER; ++i)
        {
            pid = fork();

            // 标明每一台服务器唯一的ID
            serCount++;

            // 每台服务器端口以8000为基准，逐个递增
            port++;

            if ( pid < 0 )
            {
                cout << "server fork error!!" << endl;
            }
            else if ( pid == 0 )
            {
                LinkClient(port, ip, serCount);
                exit(0);
            }
        }
        // 打印创建服务器集群的父进程的信息
        cout << "[server parent]: pid = " << getpid() << endl;
        cout << endl;
    }

private:
	char _currentTime[256];  // 系统当前时间信息
};

// 读取配置文件信息进行IP和端口配置
void ReadConfigure()
{
    FILE *readfd = fopen("server_myxml.txt", "rb");
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
                MAX_SERVER = temp[i++] - '0';
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

    Server serverRun;
    serverRun.MultiProcess();

    exit(0);
}
