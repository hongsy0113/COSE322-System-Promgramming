struct q_entry
{	
	const char *fsys_name;
	unsigned long blk_num;
	long long timestamp_sec;
	long timestamp_msec;
};

/* 작성자 : 윤상준
   작성일 20-10-29
   파일시스템의 정보와 타임스탬프를 저장하기 위한 q_entry 구조체 정의
 */

