//수정 필요한부분 *******로 표시함

#include "pch.h"
#include "tipsware.h"
//#include <stdio.h>  // fopen_s, fgets, fclose 함수를 사용하기 위해!
#include <stdlib.h> // sprintf_s, atoi, malloc, free 함수를 사용하기 위해!
#include <string.h>
#include <time.h>
#pragma warning(disable:4996)

#define MAX_NAME_LEN   16     // 이름의 최대 길이

#define ID_LISTBOX     1000   // ListBox의 ID
#define ID_ADD_BTN     1001   // '추가' 버튼의 ID
#define ID_MODIFY_BTN  1002   // '변경' 버튼의 ID
#define ID_DEL_BTN     1003   // '삭제' 버튼의 ID
#define ID_RESET_BTN   1004   // '리셋' 버튼의 ID
#define ID_SAVE_BTN    1005   // '저장' 버튼의 ID
#define ID_EXECUTE_BTN 1006   // '실행' 버튼의 ID

#define ID_ExamYear_EDIT    1010    // '시험날짜(년)' 에디트의 ID
#define ID_ExamMonth_EDIT    1011   // '시험날짜(월)' 에디트의 ID
#define ID_ExamDay_EDIT    1012     // '시험날짜(일)' 에디트의 ID
#define ID_ExamHour_EDIT    1013    // '시험시작시간(시)' 에디트의 ID
#define ID_ExamMin_EDIT    1014     // '시험시작시간(분)' 에디트의 ID
#define ID_NAME_EDIT   1015         // '이름' 에디트의 ID
#define ID_TestLength_EDIT    1016   // '시험시간' 에디트의 ID
#define ID_RequiredTime_EDIT    1017   // '공부 필요시간' 에디트의 ID
#define ID_Credit_EDIT   1018       // '학점' 에디트의 ID

typedef struct CourseData  // 시험과목 1개의 정보를 저장할 구조체 선언
{
	char name[20];
	struct tm examTime; //tm : time.h 헤더에 있는 시간을 표현할 수 있는 구조체
	int testLength;      //단위 : 분
	int requiredTime;   //단위 : 분
	int credit;         // 학점 : ex 2학점
	float importance;
} C_DATA;
typedef struct node { // schdules of exams and studying
	struct node* next; // 뒷 노드
	struct node* prev;   // 앞 노드
	char name[20];
	char type[10];
	struct tm time;
	int lastingTime;
	int requiredTime;
	float importance;
	int priority;
} node;         // 스케줄을 표현할 연결리스트에 쓰일 구조체 ***
// 프로그램이 종료될 때 사용자가 추가로 할당해서 사용하던 메모리를
// 해제할 수 있도록 자동으로 호출되어지는 함수. (CMD_USER_MESSAGE의 두 번째에 지정)
struct tm* now;
void DestoryCourseData()
{
	C_DATA* p_data;
	void* p_list_box = FindControl(ID_LISTBOX);  // 리스트 박스 컨트롤의 주소를 얻는다.
	int count = ListBox_GetCount(p_list_box); // 리스트 박스에 추가된 항목의 수를 얻는다.
	// 리스트 박스의 각 항목에 추가된 메모리를 모두 제거한다.
	for (int i = 0; i < count; i++) {
		// i 번째 항목에 저장된 주소를 가져온다.
		p_data = (C_DATA*)ListBox_GetItemDataPtr(p_list_box, i);
		free(p_data);  // p_data가 가리키는 메모리를 해제한다.
	}
	ListBox_ResetContent(p_list_box);  // 리스트 박스의 모든 항목을 제거한다.
}

float SumImportance(void* ap_list_box)
{
	C_DATA* p_data;
	int count = ListBox_GetCount(ap_list_box);
	float sum = 0;  // 리스트 박스에 추가된 항목의 수를 얻는다.
	// 리스트 박스에 추가된 학생별 총점을 모두 더한다.
	for (int i = 0; i < count; i++) {
		// i 번째 항목에 저장된 주소를 가져온다.
		p_data = (C_DATA*)ListBox_GetItemDataPtr(ap_list_box, i);
		sum += p_data->importance * 100000;  // 각 학생의 총점을 합산한다.
	}
	return sum;
}

void sleepallocation(node* head, int num)
{
	node* cur = head;
	while (1)
	{
		if (cur->next == NULL)
			break;
		tm timeTmp;
		timeTmp.tm_year = cur->time.tm_year;
		timeTmp.tm_mon = cur->time.tm_mon;
		timeTmp.tm_mday = cur->time.tm_mday;
		timeTmp.tm_hour = cur->time.tm_hour;
		timeTmp.tm_min = cur->time.tm_min + cur->lastingTime;
		timeTmp.tm_sec = 0;
		mktime(&timeTmp);
		if (timeTmp.tm_mday < cur->next->time.tm_mday)
		{
			node* temp = (node*)malloc(sizeof(node));
			strcpy(temp->type, "sleep");
			temp->lastingTime = 480;   // 단위 :분
			temp->requiredTime = 0;
			temp->priority = num;
			temp->time.tm_year = timeTmp.tm_year;
			temp->time.tm_mon = timeTmp.tm_mon;
			temp->time.tm_mday = timeTmp.tm_mday + 1;
			temp->time.tm_hour = 0;
			temp->time.tm_min = 0;
			temp->time.tm_sec = 0;
			mktime(&temp->time);
			temp->prev = cur;
			temp->next = cur->next;
			cur->next->prev = temp;
			cur->next = temp;
		}
		cur = cur->next;
	}

}
void examAllocation(node* head, void* ap_list_box, int num)
{
	C_DATA* p_data;
	time_t time1, time2;

	for (int i = 0; i < num; i++)
	{
		p_data = (C_DATA*)ListBox_GetItemDataPtr(ap_list_box, i);
		node* temp = (node*)malloc(sizeof(node));
		//copy information from courseinfo to schedule node 'temp'
		strcpy(temp->name, p_data->name);
		strcpy(temp->type, "Exam");
		temp->time.tm_year = p_data->examTime.tm_year - 1900;
		temp->time.tm_mon = p_data->examTime.tm_mon - 1;
		temp->time.tm_mday = p_data->examTime.tm_mday;
		temp->time.tm_hour = p_data->examTime.tm_hour;
		temp->time.tm_min = p_data->examTime.tm_min;
		temp->time.tm_sec = 0;
		temp->lastingTime = p_data->testLength;      // 단위 :분
		temp->requiredTime = p_data->requiredTime;   // 단위 :분
		temp->importance = p_data->importance;
		temp->priority = -1;
		time1 = mktime(&temp->time);
		//temp 노드 생성, temp 노드에 입력받은 과목의 시험시간을 입력한다

		node* cur = head;      //cur : 현재 temp와 어느것이 더 시간상 가까운지 비교중인 node
		while (1)
		{
			time2 = mktime(&cur->time);
			// temp가 시간상 더 가까우면 cur->prev와 cur 사이 삽입
			if (difftime(time1, time2) < 0)
			{
				temp->prev = cur->prev;
				temp->next = cur;
				cur->prev->next = temp;
				cur->prev = temp;
				break;
			}
			//cur 이 시간상 더 가까울때 다음 노드가 비어있으면 해당 자리에 temp 삽입
			else if (cur->next == NULL)
			{
				temp->next = NULL;
				temp->prev = cur;
				cur->next = temp;
				break;
			}
			//그마저도 아니면 다음 노드로 이동
			else
			{
				cur = cur->next;
			}

		}

	}
}
void sleepAllocation(node* head, int num)
{
	node* cur = head;
	while (1)
	{
		if (cur->next == NULL)
			break;
		tm timeTmp;
		timeTmp.tm_year = cur->time.tm_year;
		timeTmp.tm_mon = cur->time.tm_mon;
		timeTmp.tm_mday = cur->time.tm_mday;
		timeTmp.tm_hour = cur->time.tm_hour;
		timeTmp.tm_min = cur->time.tm_min + cur->lastingTime;
		timeTmp.tm_sec = 0;
		mktime(&timeTmp);
		if (timeTmp.tm_mday < cur->next->time.tm_mday)
		{
			node* temp = (node*)malloc(sizeof(node));
			strcpy(temp->type, "sleep");
			temp->lastingTime = 480;   // 단위 :분
			temp->requiredTime = 0;
			temp->priority = num;
			temp->time.tm_year = timeTmp.tm_year;
			temp->time.tm_mon = timeTmp.tm_mon;
			temp->time.tm_mday = timeTmp.tm_mday + 1;
			temp->time.tm_hour = 0;
			temp->time.tm_min = 0;
			temp->time.tm_sec = 0;
			mktime(&temp->time);
			temp->prev = cur;
			temp->next = cur->next;
			cur->next->prev = temp;
			cur->next = temp;
		}
		cur = cur->next;
	}

}

int checkUrgent(node* head, int num)
{
	int check = 0;      // 급한 과목 수를 카운트할 변수
	int sumOfRequiredTime = 0;      // 공부+ 시험보는데 필요한 시간의 합을 나타내는 변수
	time_t time1, time2;
	time2 = mktime(&head->time);
	node* cur = head;
	for (int i = 0; i < num; i++)
	{
		cur = cur->next;
		if (strcmp(cur->type, "sleep") == 0)
		{
			sumOfRequiredTime += cur->lastingTime;
			i--;
		}
		else
		{
			sumOfRequiredTime += cur->requiredTime;
			time1 = mktime(&cur->time);
			//※ 한과목이 100시간 공부해야한다 할때, 그뒤 시험 과목까지 긴급하게 만든다.-> 어떤 과목이 긴급하다 판단되었으면 그뒤는 독립적으로 생각하게 짠다
			if (difftime(time1, time2) < sumOfRequiredTime * 60) {
				check = i + 1;
				sumOfRequiredTime = 0;
				time2 = mktime(&cur->time);
			}
			sumOfRequiredTime += cur->lastingTime; // 시험시간도 감안해야함
		}
	}
	return check;
}

void priorityAllocation(node* head, int num)
{
	int diff;
	time_t time1, time2;
	node* cur;
	//중요도 순위 매기기
	float max;
	for (int i = 0; i < num; i++)
	{
		max = 0;
		cur = head;
		for (int j = 0; j < num; j++)
		{
			cur = cur->next;
			if (strcmp(cur->type, "sleep") == 0)
			{
				j--;
				continue;
			}
			if ((cur->importance > max) && (cur->priority < 0))
				max = cur->importance;
		}
		while (1)
		{
			if ((max == cur->importance) && (cur->priority < 0))
			{
				cur->priority = i;
				break;
			}
			else
				cur = cur->prev;
		}
	}
	//중요도 높은 순부터 스케줄 할당하기
	for (int i = 0; i < num; i++)   //num : 긴급한 과목수 만큼 반복
	{
		cur = head;
		while (1)
		{
			cur = cur->next;
			if (cur->priority == i)   //중요도가 가장 높은 노드 발견
			{
				node* temp = (node*)malloc(sizeof(node));
				strcpy(temp->name, cur->name);
				strcpy(temp->type, "Study");
				temp->lastingTime = cur->requiredTime;   // 단위 :분
				temp->requiredTime = 0;
				temp->priority = i;
				cur->requiredTime = 0;
				while (1)
				{
					time2 = mktime(&cur->prev->time);
					time1 = mktime(&cur->time);
					diff = (int)difftime(time1, time2) - cur->prev->lastingTime * 60;
					if (diff > 0)
					{
						temp->time.tm_year = cur->prev->time.tm_year;
						temp->time.tm_mon = cur->prev->time.tm_mon;
						temp->time.tm_mday = cur->prev->time.tm_mday;
						temp->time.tm_hour = cur->prev->time.tm_hour;
						temp->time.tm_min = cur->prev->time.tm_min + cur->prev->lastingTime;
						temp->time.tm_sec = 0;
						mktime(&temp->time);
						// cur->prev 와 cur 사이에 temp 삽입
						temp->prev = cur->prev;
						temp->next = cur;
						cur->prev->next = temp;
						cur->prev = temp;
						// 시간이 겹치는지 체크
						if (diff < temp->lastingTime * 60)
						{
							temp->requiredTime = temp->lastingTime - diff / 60;
							temp->lastingTime = diff / 60;
							i--;
						}
						break;
					}
					else if (cur->prev->prev == NULL)
						break;
					else
						cur = cur->prev;
				}
				break;
			}
		}
	}
}

void timeOrderAllocation(node* head)
{
	int diff;
	time_t time1, time2;
	node* cur = head;
	//required time 이 남아있는 node 탐색
	while (1)
	{
		cur = cur->next;
		if (cur->requiredTime != 0 && cur->priority == -1)
		{
			node* temp = (node*)malloc(sizeof(node));
			strcpy(temp->name, cur->name);
			strcpy(temp->type, "Study");
			temp->priority = -1;
			temp->lastingTime = cur->requiredTime;   // 단위 :분
			temp->requiredTime = 0;
			cur->requiredTime = 0;

			cur = head;
			while (1)
			{
				cur = cur->next;
				time2 = mktime(&cur->prev->time);
				time1 = mktime(&cur->time);
				diff = (int)difftime(time1, time2) - cur->prev->lastingTime * 60;
				if (diff > 0)
				{
					temp->time.tm_year = cur->prev->time.tm_year;
					temp->time.tm_mon = cur->prev->time.tm_mon;
					temp->time.tm_mday = cur->prev->time.tm_mday;
					temp->time.tm_hour = cur->prev->time.tm_hour;
					temp->time.tm_min = cur->prev->time.tm_min + cur->prev->lastingTime;
					temp->time.tm_sec = 0;
					mktime(&temp->time);
					// cur->prev 와 cur 사이에 temp 삽입
					temp->prev = cur->prev;
					temp->next = cur;
					cur->prev->next = temp;
					cur->prev = temp;
					// 시간이 겹치는지 체크
					if (diff < temp->lastingTime * 60)
					{
						temp->requiredTime = temp->lastingTime - diff / 60;
						temp->lastingTime = diff / 60;
					}
					break;
				}
				else if (cur->next == NULL)
					break;
			}
			cur = head;
		}
		else if (cur->next == NULL)
			break;
	}


}

void printSchedule(node* head)
{
	node* cur = head;
	struct tm start;
	struct tm end;
	int count = 0;
	int Ystart = 30;//출력값 첫 y 좌표
	int gap = 25;//행들의 간격

	Rectangle(700, 20, 1250, 345, RGB(122, 137, 164), RGB(92, 107, 134));
	TextOut(800, Ystart + gap * count++, RGB(228, 228, 228), "현재 시각 : %02d/%02d/%02d %02d:%02d", head->time.tm_year - 100, head->time.tm_mon + 1, head->time.tm_mday, head->time.tm_hour, head->time.tm_min);
	TextOut(800, Ystart + gap * count++, RGB(228, 228, 228), "===================================================");
	TextOut(800, Ystart + gap * count++, RGB(228, 228, 228), "< Schedule >");


	while (1)
	{
		cur = cur->next;
		if (strcmp(cur->type, "sleep") == 0)
			continue;
		start = cur->time;
		end.tm_year = start.tm_year;
		end.tm_mon = start.tm_mon;
		end.tm_mday = start.tm_mday;
		end.tm_hour = start.tm_hour;
		end.tm_min = start.tm_min + cur->lastingTime;
		end.tm_sec = 0;
		mktime(&end);
		TextOut(800, Ystart + gap * count, RGB(228, 228, 228), "%02d/%02d/%02d %02d:%02d - ", start.tm_year - 100, start.tm_mon + 1, start.tm_mday, start.tm_hour, start.tm_min);
		TextOut(900, Ystart + gap * count++, RGB(228, 228, 228), "%02d/%02d/%02d %02d:%02d : %s %s", end.tm_year - 100, end.tm_mon + 1, end.tm_mday, end.tm_hour, end.tm_min, cur->type, cur->name);
		if (cur->next == NULL)
			break;
	}
	TextOut(800, Ystart + gap * count++, RGB(228, 228, 228), "===================================================");
	ShowDisplay();
}
void scheduling(void* ap_list_box)
{
	C_DATA* p_data;
	//   receive the current time


	int numOfCourse = ListBox_GetCount(ap_list_box);

	// 받은 정보 일부 수정 (년, 월, 초)
	for (int i = 0; i < numOfCourse; i++)
	{
		p_data = (C_DATA*)ListBox_GetItemDataPtr(ap_list_box, i);
		p_data->examTime.tm_sec = 0;
	}
	//   Declaration of the start of the schedule
	node* head = (node*)malloc(sizeof(node));// 스케줄을 표현할 연결리스트(linked list) 의 head 선언과 head에 현재시간 
	head->next = NULL;
	head->prev = NULL;
	head->time.tm_year = now->tm_year;
	head->time.tm_mon = now->tm_mon;
	head->time.tm_mday = now->tm_mday;
	head->time.tm_hour = now->tm_hour;
	head->time.tm_min = now->tm_min + 1;
	head->time.tm_sec = 0;
	head->lastingTime = 0;

	//Starting from the test time allocating
	examAllocation(head, ap_list_box, numOfCourse);   // 시험 일정부터 스케줄에 기록하는 함수
	sleepAllocation(head, numOfCourse);
	int urgent = checkUrgent(head, numOfCourse);
	if (urgent > 0)
		priorityAllocation(head, urgent);
	timeOrderAllocation(head);
	//스케줄 출력하기
	printSchedule(head);
}
// ListBox에 선택된 학생의 정보를 에디트 컨트롤에 표시한다.
void ChangeCourseData(void* ap_list_box)
{
	int index = ListBox_GetCurSel(ap_list_box); // ListBox에서 선택된 항목의 위치 값을 얻는다.
	if (index != LB_ERR) { // 항목이 정상적으로 선택된 상황이라면!
	   // index 번째 항목에 저장된 주소를 가져온다.
		C_DATA* p_data = (C_DATA*)ListBox_GetItemDataPtr(ap_list_box, index);
		SetCtrlName(FindControl(ID_NAME_EDIT), p_data->name);            // 과목명을 표시
		SetIntDataToControl(ID_ExamYear_EDIT, p_data->examTime.tm_year);   // 시험날짜(년)을 표시
		SetIntDataToControl(ID_ExamMonth_EDIT, p_data->examTime.tm_mon);   // 시험날짜(월)을 표시
		SetIntDataToControl(ID_ExamDay_EDIT, p_data->examTime.tm_mday);      // 시험날짜(일)을 표시
		SetIntDataToControl(ID_ExamHour_EDIT, p_data->examTime.tm_hour);   // 시험시작시간(시)을 표시
		SetIntDataToControl(ID_ExamMin_EDIT, p_data->examTime.tm_min);      // 시험시작시간(분)을 표시
		SetIntDataToControl(ID_TestLength_EDIT, p_data->testLength);      // 시험시간 표시
		SetIntDataToControl(ID_RequiredTime_EDIT, p_data->requiredTime);    // 공부 필요시간 표시
		SetIntDataToControl(ID_Credit_EDIT, p_data->credit);            // 학점 표시
	}
}

// ListBox에 선택된 학생의 정보를 삭제한다.
void DeleteUserData()
{
	void* p_list_box = FindControl(ID_LISTBOX); // 리스트 박스 컨트롤의 주소를 얻는다.
	int index = ListBox_GetCurSel(p_list_box); // ListBox에서 선택된 항목의 위치 값을 얻는다.
	if (index != LB_ERR) { // 항목이 정상적으로 선택된 상황이라면!
	   // index 번째 항목에 저장된 주소를 가져온다.
		C_DATA* p_data = (C_DATA*)ListBox_GetItemDataPtr(p_list_box, index);
		char str[64];
		strcpy(str, p_data->name);
		// 선택된 항목의 정보를 사용자가 확인할 수 있도록 문자열을 구성한다.
  //      sprintf_s(str, 64, "%s", p_data->name);         *********************************************오버로드 오류나서 지워봄
		// 메시지 박스를 화면에 출력해서 사용자가 삭제를 선택하게 한다.
		if (IDOK == MessageBox(gh_main_wnd, str, "선택한 항목을 삭제하시겠습니까?",		MB_ICONQUESTION | MB_OKCANCEL)) 
		{
			ListBox_DeleteString(p_list_box, index); // ListBox에서 선택한 항목을 삭제한다.
			free(p_data); // 항목에 연결되어 있던 메모리도 해제한다.
			ShowDisplay(); // 정보를 윈도우에 출력한다.
		}
	}
}
void inputException()
{
	char str[64] = "잘못된 입력입니다. 다시 시도하세요.";
	if (IDOK == MessageBox(gh_main_wnd, str, "Error!",	MB_ICONQUESTION | MB_OK))
		ShowDisplay(); // 정보를 윈도우에 출력한다
}
// 에디트 컨트롤에 입력된 값을 시험과목 정보가 저장된 메모리에 복사한다.
int CopyUserDataFromControl(C_DATA* ap_data)
{
	struct tm temp;
	temp.tm_year = GetIntDataFromControl(ID_ExamYear_EDIT) - 1900;  // 시험날짜(년) 대입
	temp.tm_mon = GetIntDataFromControl(ID_ExamMonth_EDIT) - 1;  // 시험날짜(월) 대입
	temp.tm_mday = GetIntDataFromControl(ID_ExamDay_EDIT);  // 시험날짜(일) 대입
	temp.tm_hour = GetIntDataFromControl(ID_ExamHour_EDIT);  // 시험시작시간(시) 대입
	temp.tm_min = GetIntDataFromControl(ID_ExamMin_EDIT);  // 시험시작시간(분) 대입
	temp.tm_sec = 0;
	if (temp.tm_mon < 0 || temp.tm_mon > 11 || temp.tm_mday < 1 || temp.tm_mday>31 || temp.tm_hour < 0 || temp.tm_hour>23 || temp.tm_min < 0 || temp.tm_min>59)
	{
		inputException();
		return 0;
	}
	time_t time1, time2;
	time2 = mktime(now);
	time1 = mktime(&temp);
	if ((int)difftime(time1, time2) < 0)
	{
		inputException();
		return 0;
	}
	GetCtrlName(FindControl(ID_NAME_EDIT), ap_data->name, 16); // 과목명 대입
	ap_data->examTime.tm_year = GetIntDataFromControl(ID_ExamYear_EDIT);  // 시험날짜(년) 대입
	ap_data->examTime.tm_mon = GetIntDataFromControl(ID_ExamMonth_EDIT);  // 시험날짜(월) 대입
	ap_data->examTime.tm_mday = GetIntDataFromControl(ID_ExamDay_EDIT);  // 시험날짜(일) 대입
	ap_data->examTime.tm_hour = GetIntDataFromControl(ID_ExamHour_EDIT);  // 시험시작시간(시) 대입
	ap_data->examTime.tm_min = GetIntDataFromControl(ID_ExamMin_EDIT);  // 시험시작시간(분) 대입
	ap_data->examTime.tm_sec = 0;
	ap_data->testLength = GetIntDataFromControl(ID_TestLength_EDIT);   // 시험시간 대입
	ap_data->requiredTime = GetIntDataFromControl(ID_RequiredTime_EDIT);   // 공부 필요시간 대입
	ap_data->credit = GetIntDataFromControl(ID_Credit_EDIT); // 학점 대입
	ap_data->importance = (float)ap_data->credit / ap_data->requiredTime;  // 과목중요도 계산
	return 1;
}
// 구성된 시험과목 정보를 ListBox에 새로운 항목으로 추가하는 함수
void AddUserDataToListBox(void* ap_list_box, C_DATA* ap_user_data)
{
	char name_str[32];
	// 정렬의 기준으로 사용하기 위해 과목명을 문자열로 구성한다.
 //   sprintf_s(name_str, 32, "%s", ap_user_data->name);      **************************잠깐 오버로드 오류나서 지워봄
	// 리스트 박스에 과목명을 기준으로 정렬해서 새 항목을 추가한다.
	int index = ListBox_AddString(ap_list_box, name_str, 0);
	// 추가된 항목에 시험과목 정보가 저장된 메모리 주소를 대입한다.
	ListBox_SetItemDataPtr(ap_list_box, index, ap_user_data);
	// 추가된 위치에 선택을 표시한다.
	ListBox_SetCurSel(ap_list_box, index);
}

void AddUserData()  // '추가' 버튼을 눌렀을 때 호출되는 함수!
{
	void* p_list_box = FindControl(ID_LISTBOX); // 리스트 박스 컨트롤의 주소를 얻는다.
	// 시험과목 하나의 정보를 저장할 메모리를 할당한다.
	C_DATA* p_data = (C_DATA*)malloc(sizeof(C_DATA));
	if (CopyUserDataFromControl(p_data) == 1)// 컨트롤에 입력된 시험과목 정보를 p_data에 복사한다.
		AddUserDataToListBox(p_list_box, p_data); // 리스트 박스에 새 항목을 추가한다.
	ShowDisplay(); // 정보를 윈도우에 출력한다.
}

void ModifyUserData() // '변경' 버튼을 눌렀을 때 호출되는 함수!
{
	void* p_list_box = FindControl(ID_LISTBOX); // 리스트 박스 컨트롤의 주소를 얻는다.
	int index = ListBox_GetCurSel(p_list_box); // ListBox에서 선택된 항목의 위치 값을 얻는다.
	if (index != LB_ERR) { // 항목이 정상적으로 선택된 상황이라면!
	   // index 번째 항목에 저장된 주소를 가져온다.
		C_DATA* p_data = (C_DATA*)ListBox_GetItemDataPtr(p_list_box, index);
		// 과목명이 변경되면 ListBox에서 현재 항목의 위치를 재조정이 필요하기 때문에
		// 과목명 체크를 위해 기존 과목명을 보관한다.
		char old_name[15];
		strcpy(old_name, p_data->name);
		// 컨트롤에 입력된 과목 정보를 p_data에 복사한다.
		CopyUserDataFromControl(p_data);
		// 과목명이 동일하다면 그냥 현재 선택된 항목을 다시 선택해서
		// 변경된 값이 출력되도록 한다.
		if (strcmp(old_name, p_data->name)) ListBox_SetCurSel(p_list_box, index);
		else {
			// 과목명이 달라졌다면 기존 항목을 삭제하고 다시 추가한다.
			ListBox_DeleteString(p_list_box, index);  // 기존 항목 삭제
			AddUserDataToListBox(p_list_box, p_data); // 시험과목 정보 추가
		}
		ShowDisplay(); // 정보를 윈도우에 출력한다.
	}
}

void ResetInputControl()  /// '리셋' 버튼을 눌렀을 때 호출되는 함수!
{
	// 다섯 개의 에디트 컨트롤에 입력된 값을 모두 제거한다.
	SetCtrlName(FindControl(ID_ExamYear_EDIT), "");
	SetCtrlName(FindControl(ID_ExamMonth_EDIT), "");
	SetCtrlName(FindControl(ID_ExamDay_EDIT), "");
	SetCtrlName(FindControl(ID_ExamHour_EDIT), "");
	SetCtrlName(FindControl(ID_ExamMin_EDIT), "");
	SetCtrlName(FindControl(ID_NAME_EDIT), "");
	SetCtrlName(FindControl(ID_TestLength_EDIT), "");
	SetCtrlName(FindControl(ID_RequiredTime_EDIT), "");
	SetCtrlName(FindControl(ID_Credit_EDIT), "");
}



// 컨트롤을 조작했을 때 호출할 함수 만들기
// 컨트롤의 아이디(a_ctrl_id), 컨트롤의 조작 상태(a_notify_code), 선택한 컨트롤 객체(ap_ctrl)
void OnCommand(INT32 a_ctrl_id, INT32 a_notify_code, void* ap_ctrl)
{
	if (a_ctrl_id >= ID_ExamYear_EDIT && a_ctrl_id <= ID_Credit_EDIT) {  // 에디트 컨트롤이 조작된 경우!
		if (a_notify_code == 1000) {  // 에디트 박스에서 '엔터' 키를 누른 경우!
		   // 다음 위치에 있는 에디트 컨트롤의 번호를 계산한다.
			int next_index = ((a_ctrl_id - ID_ExamYear_EDIT) + 1) % 9;
			// 입력 포커스를 다음 컨트롤로 옮긴다.
			SetFocus(FindControl(ID_ExamYear_EDIT + next_index));
		}
	}
	else if (a_ctrl_id == ID_LISTBOX) { // ListBox 컨트롤이 조작된 경우
	 // ListBox의 항목을 선택한 경우!
		if (a_notify_code == LBN_SELCHANGE) ChangeCourseData(ap_ctrl);
	}
	else if (a_ctrl_id == ID_ADD_BTN) AddUserData();  // '추가' 버튼을 누른 경우!
	else if (a_ctrl_id == ID_MODIFY_BTN) ModifyUserData();  // '변경' 버튼을 누른 경우!
	else if (a_ctrl_id == ID_DEL_BTN) DeleteUserData();  // '삭제' 버튼을 누른 경우!
	else if (a_ctrl_id == ID_RESET_BTN) ResetInputControl();  // '리셋' 버튼을 누른 경우!
	else if (a_ctrl_id == ID_EXECUTE_BTN) scheduling(FindControl(ID_LISTBOX));  // '실행' 버튼을 누른 경우!
	//else if (a_ctrl_id == ID_SAVE_BTN) SaveUserData("data.csv"); // '저장' 버튼을 누른 경우!***************
}

// 이 프로그램이 종료될 때 DestoryStudentData 함수가 호출되도록 설정한다.
CMD_USER_MESSAGE(OnCommand, DestoryCourseData, NULL)



// 각 학생의 성적을 출력할 함수. 이 함수는 ListBox에 추가된 항목 수만큼 호출된다.
void DrawScoreItem(int index, char* ap_str, int a_str_len, void* ap_data, int a_is_selected, RECT* ap_rect)
{
	// 리스트 박스의 각 항목이 선택되었을 때와 아닐 때의 항목 테두리 색상을 다르게 처리함!
	if (a_is_selected) SelectPenObject(RGB(200, 255, 255));  // 선택 됨
	else SelectPenObject(RGB(62, 77, 104));  // 선택 안됨

	SelectBrushObject(RGB(62, 77, 104));  // 각 항목의 배경을 색상을 지정한다.
	// 각 항목의 배경으로 사용할 사각형을 그린다.
	Rectangle(ap_rect->left, ap_rect->top, ap_rect->right, ap_rect->bottom);
	// 각 항목을 구분하기 위해 구분선(점선)을 출력한다.
	Line(ap_rect->left, ap_rect->bottom - 2, ap_rect->right, ap_rect->bottom - 2,
		RGB(200, 200, 200), 2, PS_DOT | PS_GEOMETRIC);

	C_DATA* p_data = (C_DATA*)ap_data; // 현재 항목에 연결된 학생 정보를 사용하기 위해 형변환을 한다.

	SelectFontObject("굴림체", 12);  // 글꼴을 '굴림', 12 크기로 설정한다.
	TextOut(ap_rect->left + 10, ap_rect->top + 5, RGB(200, 232, 255), p_data->name); // 과목명 출력
	TextOut(ap_rect->left + 90, ap_rect->top + 5, RGB(228, 228, 228), "%d년%d월%d일", p_data->examTime.tm_year, p_data->examTime.tm_mon, p_data->examTime.tm_mday); // 시험날짜 출력
	if (p_data->examTime.tm_hour < 10 && p_data->examTime.tm_min < 10)
		TextOut(ap_rect->left + 210, ap_rect->top + 5, RGB(228, 228, 228), "0%d:0%d", p_data->examTime.tm_hour, p_data->examTime.tm_min); //시험시작시간 출력
	else if (p_data->examTime.tm_min < 10)
		TextOut(ap_rect->left + 210, ap_rect->top + 5, RGB(228, 228, 228), "%d:0%d", p_data->examTime.tm_hour, p_data->examTime.tm_min);
	else if (p_data->examTime.tm_hour < 10)
		TextOut(ap_rect->left + 210, ap_rect->top + 5, RGB(228, 228, 228), "0%d:%d", p_data->examTime.tm_hour, p_data->examTime.tm_min);
	else
		TextOut(ap_rect->left + 210, ap_rect->top + 5, RGB(228, 228, 228), "%d:%d", p_data->examTime.tm_hour, p_data->examTime.tm_min);
	TextOut(ap_rect->left + 315, ap_rect->top + 5, RGB(228, 228, 228), "%d", p_data->testLength); // 시험소요시간 출력
	TextOut(ap_rect->left + 400, ap_rect->top + 5, RGB(200, 232, 255), "%d", p_data->requiredTime); // 공부필요시간 출력
	TextOut(ap_rect->left + 478, ap_rect->top + 5, RGB(100, 228, 100), "%d", p_data->credit); // 학점 출력


  // 개인 평균 성적을 막대 그래프로 출력하기 위해 배경을 먼저 그리고 그 위에 자신의 평균만큼 사각형을 그린다.
	Rectangle(ap_rect->left + 517, ap_rect->top + 2, ap_rect->left + 500 + 101,
		ap_rect->bottom - 2, RGB(128, 128, 0), RGB(64, 64, 0)); // 배경!!
	float temp1 = SumImportance(FindControl(ID_LISTBOX));
	int temp = int(p_data->importance * 100000 / temp1 * 100);
	Rectangle(ap_rect->left + 518, ap_rect->top + 3, ap_rect->left + 501 + temp,
		ap_rect->bottom - 3, RGB(255, 255, 0), RGB(200, 200, 0)); // 과목중요도 크기를 막대기로 나타냄
}

int main()
{
	time_t timer = time(NULL);
	now = localtime(&timer);

	ChangeWorkSize(1300, 365); // 작업 영역을 설정한다.

	Clear(0, RGB(72, 87, 114)); // 윈도우의 배경을 RGB(72, 87, 114) 색으로 채운다!

	// 타이틀이 출력될 영역에 사각형을 그린다.
	Rectangle(10, 10, 645, 35, RGB(122, 137, 164), RGB(92, 107, 134));
	SelectFontObject("굴림체", 12);  // 글꼴을 '굴림체', 12 크기로 설정한다.
	TextOut(34, 18, RGB(228, 228, 228), "과목명       시험날짜      시험시작시간    시험소요시간    공부필요시간   학점        중요도 ");

	// ID_USER_LISTBOX번 아이디를 가진 리스트 박스를 생성하고
	// 각 항목을 출력할 때는 DrawScoreItem 함수를 사용한다.
	void* p = CreateListBox(10, 40, 635, 200, ID_LISTBOX, DrawScoreItem);
	SetCtrlFont(p, "굴림체", 12);  // 리스트 박스의 글꼴을 '굴림체'로 변경한다.
	ListBox_SetItemHeight(p, 20);  // 항목의 높이를 20으로 설정한다.

	//ReadFileDataToListBox(p, "data.csv"); // 파일에서 과목 정보를 읽어서 ListBox에 추가한다. *******************
	//ShowAverageScore(p);  // 학생수와 성적의 평균을 출력한다.                     ***********************

	// 프로그램에서 사용할 버튼을 생성합니다.
	CreateButton("추가", 275, 275, 58, 28, ID_ADD_BTN);
	CreateButton("변경", 335, 275, 58, 28, ID_MODIFY_BTN);
	CreateButton("삭제", 395, 275, 58, 28, ID_DEL_BTN);
	CreateButton("리셋", 525, 275, 58, 28, ID_RESET_BTN);
	//CreateButton("저장", 580, 275, 58, 28, ID_SAVE_BTN);*****************************저장살릴거면
	CreateButton("실행", 585, 275, 58, 28, ID_EXECUTE_BTN);

	// 에디트 컨트롤 위쪽에 각 에디트 컨트롤의 의미를 표시하기 위한 문자열을 출력한다.
	TextOut(34, 316, RGB(228, 228, 228),
		"과목명     시험날짜(연도/월/일)   시험시작시간(시:분)   시험소요시간(분)   공부필요시간(분)    학점");
	// 시험과목 정보 입력에 사용할 에디트 컨트롤을 생성한다.
	EnableEnterKey(CreateEdit(18, 334, 65, 20, ID_NAME_EDIT, 0));
	EnableEnterKey(CreateEdit(104, 334, 40, 20, ID_ExamYear_EDIT, 0));
	EnableEnterKey(CreateEdit(149, 334, 30, 20, ID_ExamMonth_EDIT, 0));
	EnableEnterKey(CreateEdit(184, 334, 30, 20, ID_ExamDay_EDIT, 0));
	EnableEnterKey(CreateEdit(252, 334, 40, 20, ID_ExamHour_EDIT, 0));
	EnableEnterKey(CreateEdit(297, 334, 40, 20, ID_ExamMin_EDIT, 0));
	EnableEnterKey(CreateEdit(387, 334, 50, 20, ID_TestLength_EDIT, 0));
	EnableEnterKey(CreateEdit(504, 334, 50, 20, ID_RequiredTime_EDIT, 0));
	EnableEnterKey(CreateEdit(596, 334, 40, 20, ID_Credit_EDIT, 0));
	ShowDisplay(); // 정보를 윈도우에 출력한다.
	return 0;
}