 &     
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       22222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222
                                                                                                                                                                                            333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333
33333333333333333333333333333333333333333333333333555555555555555555555555
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666
6661111111111111
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  #ifndef BLOCKS_H
#define BLOCKS_H

#define BLOCK_SIZE 512

int OpenFile(const char * path);
int CloseFile(int fd);

int ReadBlock(int fd, int block_num, void * block);
int WriteBlock(int fd, int block_num, void * block);
int BlockCounter(int fd);

int WriteFile(int fd, int block_num, const char * source);

#endif                                                                                                                                                                                                            :@     �+�    �4     �4     :@     :@     �4     @5           8    (                               P���   -%@     	:@     �4                  P+.            �A  �  �                                   �X    z    �X    v�X    �X    v�X                                         Q+.            �A  �  �                                   �X    z    �X    v�X    �X    v�X                                                                                      ����.                                  ..                                 ekf                                blocks.h                                        P+.            �A  �  �                                   �X    z    �X    v�X    �X    v�X                                         Q+.            �A  �  �                                   �X    z    �X    v�X    �X    v�X                                                                                      ����.                                  ..                                 dirA                               dirB                               file1                           
   �  �                                   �X    z    �X    v�X    �X    v�X                                         Q+.            �A  �  �                                   �X    z    �X    v�X    �X    v�X                                                                                      ����.                                  ..                                 dirC                               file2                              file3                              �  �                                   �X    z    �X    v�X    �X    v�X                                         Q+.            �A  �  �                                   �X    z    �X    v�X    �X    v�X                                                                                               Y+.            �A  �  �                                   �i�X    ���    �i�X    R�B8    �i�X    R�B8                                         V+.            ��  �  �              D                    �X    fv�    �X    v�X    �X    v�X                                         W+.            ��  �  �              �                    �X    fv�    �X    v�X    �X    v�X                                                                                      ����.                                  ..                                 �i�X    ���    �i�X    R�B8    �i�X    R�B8                                         V+.            ��  �  �              D                    �X    fv�    �X    v�X    �X    v�X                                         W+.            ��  �  �              �                    �X    fv�    �X    v�X    �X    v�X                                                                                               )8            �A  �  �                                   �X    z    �X    v�X    �X    v�X                                         )8            ��  �  �                                   �X    fv�    �X    v�X    �X    v�X                                         )8            ��  �  �              �                    �X    fv�    �X    v�X    �X    v�X                                                                                      ����.                                  ..                                 file5                              file6                           	                )8            ��  �  �                                   �X    fv�    �X    v�X    �X    v�X                                         )8            ��  �  �              �                    �X    fv�    �X    v�X    �X    v�X                                                                                      ����      X+.            ��  �  �                                   �X    fv�    �X    v�X    �X    v�X                                         B+.            ��  �  �              :                    ��X    �    >�X    �JX    ��X    ��4                            	             )8            ��  �  �              �                    �X    fv�    �X    v�X    �X    v�X                                                                                   