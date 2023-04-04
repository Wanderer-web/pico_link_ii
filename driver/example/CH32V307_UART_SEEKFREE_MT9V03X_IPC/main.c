/*********************************************************************************************************************
* CH32V307VCT6 Opensourec Library ����CH32V307VCT6 ��Դ�⣩��һ�����ڹٷ� SDK �ӿڵĵ�������Դ��
* Copyright (c) 2022 SEEKFREE ��ɿƼ�
*
* ���ļ���CH32V307VCT6 ��Դ���һ����
*
* CH32V307VCT6 ��Դ�� ��������
* �����Ը��������������ᷢ���� GPL��GNU General Public License���� GNUͨ�ù������֤��������
* �� GPL �ĵ�3�棨�� GPL3.0������ѡ��ģ��κκ����İ汾�����·�����/���޸���
*
* ����Դ��ķ�����ϣ�����ܷ������ã�����δ�������κεı�֤
* ����û�������������Ի��ʺ��ض���;�ı�֤
* ����ϸ����μ� GPL
*
* ��Ӧ�����յ�����Դ���ͬʱ�յ�һ�� GPL �ĸ���
* ���û�У������<https://www.gnu.org/licenses/>
*
* ����ע����
* ����Դ��ʹ�� GPL3.0 ��Դ���֤Э�� �����������Ϊ���İ汾
* �������Ӣ�İ��� libraries/doc �ļ����µ� GPL3_permission_statement.txt �ļ���
* ���֤������ libraries �ļ����� �����ļ����µ� LICENSE �ļ�
* ��ӭ��λʹ�ò����������� ���޸�����ʱ���뱣����ɿƼ��İ�Ȩ����������������
*
* �ļ�����          main
* ��˾����          �ɶ���ɿƼ����޹�˾
* �汾��Ϣ          �鿴 libraries/doc �ļ����� version �ļ� �汾˵��
* ��������          MounRiver Studio V1.8.1
* ����ƽ̨          CH32V307VCT6
* ��������          https://seekfree.taobao.com/
*
* �޸ļ�¼
* ����                                      ����                             ��ע
* 2022-09-15        ��W            first version
********************************************************************************************************************/
#include "zf_common_headfile.h"
#include "pico_link_ii.h"

#define IPCH 60 // ��λ����ʾͼ��߶�
#define IPCW 80 // ��λ����ʾͼ����
#define RED 1
#define BLUE 2
#define YELLOW 3
#define GREEN 4

float Mh = MT9V03X_H;
float Lh = IPCH;
float Mw = MT9V03X_W;
float Lw = IPCW;
uint8 Image_Use[IPCH][IPCW];

void Compressimage()
{
  int i, j, row, line;
  const float div_h = Mh / Lh, div_w = Mw / Lw;
  for (i = 0; i < IPCH; i++)
  {
    row = i * div_h + 0.5;
    for (j = 0; j < IPCW; j++)
    {
      line = j * div_w + 0.5;
      Image_Use[i][j] = mt9v03x_image[row][line];

      // ��ɫ����
      //      if(i==10)
      //          Image_Use[i][j]=RED;
      //      if(i==20)
      //          Image_Use[i][j]=BLUE;
      //      if(i==30)
      //          Image_Use[i][j]=YELLOW;
      //      if(i==40)
      //          Image_Use[i][j]=GREEN;
    }
  }
  // ʹ����һ֡DMA�����ͼ��ͼ��  ���Կ�ʼ������һ֡
}

int main (void)
{
    clock_init(SYSTEM_CLOCK_144M);                                              // ��ʼ��оƬʱ�� ����Ƶ��Ϊ 144MHz
    debug_init();                                                               // ��ʼ��Ĭ�� Debug UART

    // �˴���д�û����� ���������ʼ�������
    mt9v03x_init();                                                   // ��ʼ������ͷ
    gpio_init(E8, GPI, GPIO_HIGH, GPI_PULL_UP);                       // ��ʼ��CTS��
    uart_init(UART_7, 3000000, UART7_MAP3_TX_E12, UART7_MAP3_RX_E13); // ��ʼ�����ڣ�������3000000
    // �˴���д�û����� ���������ʼ�������

    while(1)
    {
        // �˴���д��Ҫѭ��ִ�еĴ���
        if (mt9v03x_finish_flag == 1)
        {
            Compressimage();                                  // ѹ��ͼ��
            ipcSendImg_Uart((pico_uint8 *)Image_Use, 60, 80); // ����ͼ��
            mt9v03x_finish_flag = 0;
        }
        // �˴���д��Ҫѭ��ִ�еĴ���
    }
}

