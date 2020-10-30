첨부파일들에 대한 설명

blk-core.c
- 수정한 커널 소스파일 
- 큐 정의 및 write 요청 시 큐에 데이터를 저장하는 함수 정의 및 호출

myCircle-h
- 수정한 커널 헤더파일
- include/linux에 추가해서 사용함
- 파일시스템 정보와 타임스탬프를 저장하기 위한 q_entry 구조체 정의

모듈(ext4)/test2.c
- 커널 모듈 소스 파일
- ext4에 해당하는 queue 정보를 출력

모듈(ext4)/Makefile, test2.ko
- 모듈 생성을 위한 Makefile 과 컴파일 된 모듈 파일

모듈(f2fs)/test3.c
- 커널 모듈 소스 파일
- f2fs에 해당하는 queue 정보를 출력

모듈(f2fs)/Makefile, test3.ko
- 모듈 생성을 위한 Makefile 과 컴파일 된 모듈 파일

RAW_DATA
-ext4와 f2fs 테스트 후 출력된 raw 데이터
- sheet1, 2 : f2fs
- sheet 3, 4  : ext4


1차과제 보고서(sp_group4).PDF
- 과제 보고서 파일
