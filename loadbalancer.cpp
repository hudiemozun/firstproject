#include "loadbalancer.h"

#include <ctime>

// 将配置信息以全局的方式定义
// 通过函数读取配置文件进行信息配置

// 最大服务器注册数量
#define  MAX_SERVER  5
// 最大客户端链接量
int MAX_CLIENT = 0;
// 负载均衡器的端口和IP
int PORT = 0;
char IP[32] = {0};
// 服务器注册信息
int SERVER_PORT = 0;
char SERVER_IP[32] = {0};

class LoadBalancer
{
public:
    // 将所有服务器的信息注册到负载均衡器上
    LoadBalancer()
    {
        int port = SERVER_PORT;
        char ip[32] = {0};
        strcpy(ip, SERVER_IP);

        int i = 0;
        for ( ; i < MAX_SERVER; ++i )
        {
            _port[i] = port;
            port++;
        }
        strcpy(_ip, ip);
    }
    ~LoadBalancer(){}
    // 调配服务器给客户端
	void Balancing()
	{
	    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	    assert(sockfd != -1);

	    struct sockaddr_in saddr;
	    memset(&saddr, 0, sizeof(saddr));
	    saddr.sin_family = AF_INET;
	    saddr.sin_port = htons(PORT);
	    saddr.sin_addr.s_addr = inet_addr(IP);

	    int res = bind(sockfd, (struct sockaddr *)&saddr, sizeof(saddr));
	    assert(res != -1);

        // 标志位，轮询服务器，通过改变分配服务器的端口,
        // 来改变分配给客户端的每一个服务器
        int curServer = 0;
        while (1)
        {
			char recvbuff[128] = { 0 };
			int len = sizeof(saddr);

            // 用来装填分配服务器的IP和端口
            char ServerInformation[256] = {0};

            char port[16] = {0};
            sprintf(port, "%d", _port[curServer++]);

            strcpy(ServerInformation, port);
            strcat(ServerInformation, _ip);
            
            // 如果当前服务器分配达到最大服务器注册量,
            // 则从头重新开始新一遍的轮询分配
            if ( curServer == MAX_SERVER )
            {
                curServer = 0;
            }

			recvfrom(sockfd, recvbuff, 128, 0, (struct sockaddr *)&saddr, (socklen_t *)&len);

            // 打印链接的客户端信息以及分配的服务器信息
            cout << endl;
		    cout << "[client " << recvbuff << "]: request server" << endl;
		    cout << "[server]: ip = " << _ip << ",port = " << port << endl;

			sendto(sockfd, ServerInformation, strlen(ServerInformation), 0, (struct sockaddr *)&saddr, sizeof(saddr));
        }
		close(sockfd);
	}

private:
	int _port[MAX_SERVER];  // 服务器注册信息--->端口
    char _ip[32];          // 服务器注册信息--->IP
};

// 读取配置文件信息进行IP和端口配置
void ReadConfigure()
{
    FILE *readfd = fopen("loadbalancer_myxml.txt", "rb");
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
            SERVER_PORT = atoi(port);
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
                SERVER_IP[j++] = temp[i++];
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

    LoadBalancer loadbalancerRun;
    loadbalancerRun.Balancing();


    exit(0);
}
