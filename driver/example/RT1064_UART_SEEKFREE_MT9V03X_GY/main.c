/*********************************************************************************************************************
 * RT1064DVL6A Opensourec Library ����RT1064DVL6A ��Դ�⣩��һ�����ڹٷ� SDK �ӿڵĵ�������Դ��
 * Copyright (c) 2022 SEEKFREE ��ɿƼ�
 *
 * ���ļ��� RT1064DVL6A ��Դ���һ����
 *
 * RT1064DVL6A ��Դ�� ��������
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
 * ��������          IAR 8.32.4 or MDK 5.33
 * ����ƽ̨          RT1064DVL6A
 * ��������          https://seekfree.taobao.com/
 *
 * �޸ļ�¼
 * ����              ����                ��ע
 * 2022-09-21        SeekFree            first version
 ********************************************************************************************************************/

#include "zf_common_headfile.h"
#include "pico_link_ii.h"
#include "pico_GY.h"

// ���µĹ��̻��߹����ƶ���λ�����ִ�����²���
// ��һ�� �ر��������д򿪵��ļ�
// �ڶ��� project->clean  �ȴ��·�����������

// �������ǿ�Դ����ֲ�ÿչ���

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

int main(void)
{
  clock_init(SYSTEM_CLOCK_600M); // ����ɾ��
  debug_init();                  // ���Զ˿ڳ�ʼ��

  // �˴���д�û����� ���������ʼ�������
  mt9v03x_init();                                         // ��ʼ������ͷ
  gpio_init(D26, GPI, GPIO_HIGH, GPI_PULL_UP);            // ��ʼ��CTS��
  uart_init(UART_8, 3000000, UART8_TX_D16, UART8_RX_D17); // ��ʼ�����ڣ�������3000000
  // �˴���д�û����� ���������ʼ�������
  while (1)
  {
    // �˴���д��Ҫѭ��ִ�еĴ���
    if (mt9v03x_finish_flag == 1)
    {
      Compressimage();                          // ѹ��ͼ��
      sendimg((pico_uint8 *)Image_Use, 80, 60); // ����ͼ��
      mt9v03x_finish_flag = 0;
    }
    // �˴���д��Ҫѭ��ִ�еĴ���
  }
}