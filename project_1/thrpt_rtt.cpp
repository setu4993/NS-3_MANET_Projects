#include <stdio.h>
#include <time.h>
#include <pcap.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>

void my_packet_handler(u_char *args, const struct pcap_pkthdr *header, const u_char *packet);
void calc_packet_info(const u_char *packet, struct pcap_pkthdr packet_header);


//Global variables for data and time calculations
int i = 0;
double tme1 = 0;
double tme2 = 0;
double rtm1 = 0;
double rtm2 = 0;
double ftm1 = 0;
double ftm2 = 0;
double tdata = 0;
double rtt = 0;

int main(int argc, char *argv[])
{
    char error_buffer[PCAP_ERRBUF_SIZE];
    pcap_t *handle = pcap_open_offline("/home/setu/Documents/ece695/ns-3-allinone/ns-3-dev/thrpt-0-0.pcap", error_buffer);
    int timeout_limit = 10000; /* In milliseconds */
    double thrpt;
    
    if (handle == NULL)
    {
         return 2;
    }
    
    pcap_loop(handle, 0, my_packet_handler, NULL);
    pcap_close(handle);
    
    
    //Calculations of time and throughput
    ftm1 = (ftm1 + ftm2) - (rtm1 + rtm2);
    thrpt = tdata * 8 / (ftm1 * 1000000);
    rtt = (((tme1 + tme2)/i) - (rtm1 + rtm2));
    
    
    //Output
    printf("\nTotal number of packets sent / received: %d", i);
    printf("\n\nTotal data transmitted: %f", tdata);
    printf("\n\nAverage round trip time of packets: %f", rtt);
    printf("\n\nThroughput: %f Mbit/s\n\n", thrpt);
    return 0;
}

void my_packet_handler(u_char *args, const struct pcap_pkthdr *packet_header, const u_char *packet_body)
{
    calc_packet_info(packet_body, *packet_header);
    return;
}

void calc_packet_info(const u_char *packet, struct pcap_pkthdr packet_header)
{    
    //Recording data of each packet
    tdata = tdata + packet_header.len;
    i++;
    if(i == 1)
    {
		rtm1 = packet_header.ts.tv_sec;
		rtm2 = packet_header.ts.tv_usec*0.000001;
		tme1 = rtm1;
		tme2 = rtm2;
	}
	else
	{
		ftm1 = packet_header.ts.tv_sec;
		ftm2 = packet_header.ts.tv_usec*0.000001;
		tme1 = tme1 + ftm1;
		tme2 = tme2 + ftm2;
	}
}
