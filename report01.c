#include <stdio.h>

extern void get_school_position(int school_index, int* posX, int* posY);
extern void get_student_position(int student_index, int* posX, int* posY);
extern void set_student_school(int student_index, int school_index);

#define __YU_INT_MAX__ 2147483647
#define __YU_INT_MIN__ -2147483648
#define __YU_FLOAT_MAX__ 3.402823466e+38F
#define __YU_FLOAT_MIN__ -3.402823466e+38F

#define SWAP 

typedef struct s_point
{
	int x;
	int y;

	float x_f;
	float y_f;
	float dist_from_circumcenter; /* for sorting optimization */
}	t_point;

typedef struct s_student
{
	int index;
	int assigned_school;
	t_point pos;
}	t_student;

typedef struct s_school
{
	int index;
	t_point pos;
	int student_count;
}	t_school;

static const char *error_messages[] = {
	"An unknown error occurred",
	"INT_MIN cannot return an absolute value",
	"power_f function do not support negative powers",
	"failure get a sqrt val of x",
	"x is too big to get sqrt val",
	"x is too small to get sqrt val",
	"failure calculating distance"
};

const static char* get_error_message(int code) {
    if (code < 2) 
	{
        return error_messages[0];
    }
    return error_messages[code];
}

// //do not check overflow
// static float power_f(float x, int r)
// {
// 	float result;
// 	int i;

// 	if (r < 0)
// 		printf("ERROR : %s\n", get_error_message(2));
// 	for (i = 0, result = 1; i < r; i++)
// 	{
// 		result *= x;
// 	}
// 	return (result);
// }

//소수점 3자리까지만 비교하여 구함.
static float sqrt(float x)
{
	// float i;
	// float pow_val;
	// float tmp1;
	// float tmp2;

	// if (x < 1.0f)
	// 	printf("ERROR: %s\n", get_error_message(5));
	// if (x > __YU_FLOAT_MAX__/ 1000)
	// 	printf("ERROR: %s\n", get_error_message(4));
	// tmp2 = x;
	// tmp2 *= power_f(10, 3);
	// //0.001씩 더하는 건 매우 효율 나쁜 짓이긴 함.
	// //차후 다소의 수정이 필요.
	// for (i=1.0f, pow_val=1.0f; pow_val < x; i+=0.001f)
	// {
	// 	pow_val = power_f(i, 2);
	// 	tmp1 = pow_val;
	// 	tmp1 *= power_f(10, 3);
	// 	if ((int)tmp1 == (int)tmp2)
	// 		return (i);
	// }
	// printf("ERROR: %s\n", get_error_message(3));
	// return(-1.0f);

	float guess;
    float prev;

    if (x <= 0.0f)
        return 0.0f;
    
    guess = x / 2.0f;
    prev = 0.0f;
    
    // 이전 추정값과 현재 추정값이 같아질 때까지(최대 정밀도 도달) 반복
    while (guess != prev)
    {
        prev = guess;
        guess = (guess + x / guess) / 2.0f;
    }
    return guess;
}

static int abs(int x)
{
	if (x == __YU_INT_MIN__)
		printf("ERROR : %s\n", get_error_message(1));
	if (x < 0)
		return -x;
	else
		return x;
}

static float get_dist(t_point a, t_point b)
{
	float dx;
	float dy;
	float dist;

	dx = abs(b.x - a.x);
	dy = abs(b.y - a.y);
	dist = sqrt(dx*dx + dy*dy);
	if (dist < 0)
	{
		printf("ERROR: %s\n", get_error_message(6));
	}
	return (dist);
}

static void set_circumcenter_of_schools(t_point *circumcenter, t_school schools[3])
{
	/*
		Compute circumcenter of triangle (A,B,C).
		Uses standard determinant formula; if degenerate (collinear), fallback to centroid.
	*/
	float ax = schools[0].pos.x_f;
	float ay = schools[0].pos.y_f;
	float bx = schools[1].pos.x_f;
	float by = schools[1].pos.y_f;
	float cx = schools[2].pos.x_f;
	float cy = schools[2].pos.y_f;

	float a2 = ax * ax + ay * ay;
	float b2 = bx * bx + by * by;
	float c2 = cx * cx + cy * cy;

	float d = 2.0f * (ax * (by - cy) + bx * (cy - ay) + cx * (ay - by));
	/* very small determinant => nearly collinear */
	if (d < 0.000001f && d > -0.000001f)
	{
		circumcenter->x_f = (ax + bx + cx) / 3.0f;
		circumcenter->y_f = (ay + by + cy) / 3.0f;
	}
	else
	{
		circumcenter->x_f = (a2 * (by - cy) + b2 * (cy - ay) + c2 * (ay - by)) / d;
		circumcenter->y_f = (a2 * (cx - bx) + b2 * (ax - cx) + c2 * (bx - ax)) / d;
	}
	/* keep int fields consistent (best-effort rounding) */
	circumcenter->x = (int)(circumcenter->x_f);
	circumcenter->y = (int)(circumcenter->y_f);
}

// static void sort_students_far_to_near_from_circumcenter(t_point *circumcenter, t_student students[10000])
// {
// 	for (int i = 0; i < 10000; i++)
// 	{
// 		students[i].pos.dist_from_circumcenter = get_dist(*circumcenter, students[i].pos);
// 	}
// 	for (int i = 1; i < 10000; i++)
// 	{
// 		t_student key = students[i];
// 		int j = i - 1;
// 		while (j >= 0 && students[j].pos.dist_from_circumcenter < key.pos.dist_from_circumcenter)
// 		{
// 			students[j + 1] = students[j];
// 			j--;
// 		}
// 		students[j + 1] = key;
// 	}
// }

static void swap_students(t_student *a, t_student *b)
{
    t_student temp = *a;
    *a = *b;
    *b = temp;
}

static void quick_sort_students(t_student students[], int left, int right)
{
    if (left >= right) return;

    // 피벗을 중간값으로 설정 (Far to Near: 내림차순 정렬)
    float pivot = students[(left + right) / 2].pos.dist_from_circumcenter;
    int i = left;
    int j = right;

    while (i <= j)
    {
        // 내림차순이므로 피벗보다 '큰' 놈들을 왼쪽으로, '작은' 놈들을 오른쪽으로
        while (students[i].pos.dist_from_circumcenter > pivot) i++;
        while (students[j].pos.dist_from_circumcenter < pivot) j--;

        if (i <= j)
        {
            swap_students(&students[i], &students[j]);
            i++;
            j--;
        }
    }

    if (left < j) quick_sort_students(students, left, j);
    if (i < right) quick_sort_students(students, i, right);
}

static void sort_students_far_to_near_from_circumcenter(t_point *circumcenter, t_student students[10000])
{
    for (int i = 0; i < 10000; i++)
    {
        // 이전에 말씀드린 대로 sqrt를 개선하거나, 비교용이라면 제곱값만 써도 됩니다.
        students[i].pos.dist_from_circumcenter = get_dist(*circumcenter, students[i].pos);
    }
    
    // 퀵 정렬 실행
    quick_sort_students(students, 0, 9999);
}

static void allocate_students(t_school schools[3], t_student student)
{
	int best = 0;
	float best_d = get_dist(schools[0].pos, student.pos);
	for (int i = 1; i < 3; i++)
	{
		float d = get_dist(schools[i].pos, student.pos);
		if (d < best_d)
		{
			best_d = d;
			best = i;
		}
	}
	if (schools[best].student_count >= 3500)
	{
		// 이미 학생이 3500명 배정된 학교는 제외
		for (int i = 0; i < 3; i++)
		{
			if (i != best && schools[i].student_count < 3500)
			{
				float d = get_dist(schools[i].pos, student.pos);
				if (d < best_d)
				{
					best_d = d;
					best = i;
				}
			}
		}
	}
	schools[best].student_count++;
	set_student_school(student.index, best);
}

static void init_school_position(t_school school[3])
{
	get_school_position(0, &school[0].pos.x, &school[0].pos.y);
	get_school_position(1, &school[1].pos.x, &school[1].pos.y);
	get_school_position(2, &school[2].pos.x, &school[2].pos.y);

	school[0].pos.x_f = (float)school[0].pos.x;
	school[0].pos.y_f = (float)school[0].pos.y;

	school[1].pos.x_f = (float)school[1].pos.x;
	school[1].pos.y_f = (float)school[1].pos.y;

	school[2].pos.x_f = (float)school[2].pos.x;
	school[2].pos.y_f = (float)school[2].pos.y;

	// Initialize student count
	school[0].student_count = 0;
	school[1].student_count = 0;
	school[2].student_count = 0;
}

//debug functions
// void print_result(t_student students[10000], t_school schools[3])
// {
// 	static int count = 0;
// 	count++;
// 	float ave_dist = 0.0f;
// 	for (int i = 0; i < 10000; i++)
// 	{
// 		int s = students[i].assigned_school;
// 		if (s >= 0 && s < 3)
// 			ave_dist += get_dist(schools[s].pos, students[i].pos);
// 	}
// 	ave_dist /= 10000.0f;
// 	printf("======result %d======\n", count);
// 	printf("school 1의 배정된 학생의 수: %d\n", schools[0].student_count);
// 	printf("school 2의 배정된 학생의 수: %d\n", schools[1].student_count);
// 	printf("school 3의 배정된 학생의 수: %d\n", schools[2].student_count);
// 	printf("\n");
// 	printf("학생과 학교의 평균 거리: %.4f\n", ave_dist);
// 	printf("====================\n\n\n");
// }

void run_contest(void)
{
	t_point circumcenter;
	t_school schools[3];
	t_student students[10000];

	init_school_position(schools);
	set_circumcenter_of_schools(&circumcenter, schools);
	for (int i = 0; i < 10000; i++)
	{
		get_student_position(i, &students[i].pos.x, &students[i].pos.y);
		students[i].index = i;
		students[i].assigned_school = -1;
		students[i].pos.x_f = students[i].pos.x;
		students[i].pos.y_f = students[i].pos.y;
	}
	sort_students_far_to_near_from_circumcenter(&circumcenter, students);
	for (int j = 0; j < 10000; j++)	/* allocate_students takes student by value, so we set assigned_school here */
	{
		int best = 0;
		float best_d = get_dist(schools[0].pos, students[j].pos);
		for (int k = 1; k < 3; k++)
		{
			float d = get_dist(schools[k].pos, students[j].pos);
			if (d < best_d)
			{
				best_d = d;
				best = k;
			}
		}
		if (schools[best].student_count >= 3500)
		{
			for (int k = 0; k < 3; k++)
			{
				if (k != best && schools[k].student_count < 3500)
				{
					float d = get_dist(schools[k].pos, students[j].pos);
					if (d < best_d)
					{
						best_d = d;
						best = k;
					}
				}
			}
		}
		students[j].assigned_school = best;
		allocate_students(schools, students[j]);
	}
	//print_result(students, schools);
}
