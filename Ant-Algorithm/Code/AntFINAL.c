#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define PI 3.14159265359//圆周率
#define Equatorial_radius 6.378140 //地球半径，用于求两城市之间的球面距离
#define ant_num 10//蚂蚁的数量
#define city_num 10//蚂蚁访问的城市的数量

float ALPHA = 1.0;//信息素因子
float BETA = 2;//启发函数因子
float RHO = 0.3;//信息素见效的频率
float Q = 100.0;//每只蚂蚁最开始携带的信息素量
int iter = 0;
//蚂蚁——结构体
typedef struct
{
	int ID;//蚂蚁的序号
	int city_stand;//蚂蚁此时所在的城市
	int city_visited;//蚂蚁已经走过的城市的数量
	int* city_visit_path;//蚂蚁走过的路径 如经过了1，2，3号城市，则为1，2，3
	float total_distance;//蚂蚁走过的总路程
}ANT;
//城市——结构体
typedef struct CITY
{
	char* name;//城市的名称
	int visited;//1——已被访问； 0——未访问
	float city_lat;//所在纬度
	float city_lon;//所在经度
}CITY;
//地图——结构体
typedef struct Map
{
	int size_of_map;//地图中有几个城市元素
	float** pheromone_graph;//用二重指针实现二维信息素矩阵
	float** distance_graph;//用二重指针实现二维距离矩阵
}MAP;

void init_ant(ANT* ant, int id, CITY* city, int num_of_city);
void copy_ant(ANT* super_ant, ANT ant);
int random_city(int num_of_city);
float random_prob(float total_prob);
float R_total_prob(ANT* ant, CITY city[], MAP* map, int num_of_city);
void init_city(CITY* city, int num_of_city);
int select_city(ANT * ant,float total_prob, CITY* city, MAP* map);
void sum_distance(ANT* ant, MAP* map);
void ant_move(ANT* ant, int next_city, MAP* map);
void ant_search_path(ANT* ant, int id, MAP* map, int num_of_city, CITY* city);
void init_map(MAP* map);
void sphere_distance(CITY city1, CITY city2, int i, int j, MAP* map);
void map_updata_pheromone(MAP* map, int num_of_city, ANT ant[]);
void init_TSP(ANT ant[], ANT* super_ant, CITY city[], MAP* map, int num_of_city, int num_of_ant);
void search_1_TSP(ANT ant[], ANT* super_ant, CITY* city, MAP* map, int num_of_city, int num_of_ant);
void search_n_TSP(ANT ant[], ANT* super_ant, CITY* city, MAP* map, int num_of_city, int num_of_ant, int times_of_search);
void solution_TSP();



//随机初始化一只小蚂蚁
void init_ant(ANT* ant, int id, CITY* city, int num_of_city)
{
	ant->ID = id;                            //赋予编号
	ant->city_visited = 1;                   //初始化城市
	ant->total_distance = 0.0;          //访问总距离
	ant->city_visit_path = (int*)malloc(num_of_city * sizeof(int));
	//访问路径
	ant->city_stand = random_city(city_num); //随机生成初始化城市
	init_city(city, city_num);              //初始化地图
	city[ant->city_stand].visited = 1;       //将蚂蚁放在初始化城市中
	ant->city_visit_path[0] = ant->city_stand;
}

//蚂蚁——拷贝蚂蚁——深度拷贝
void copy_ant(ANT* super_ant, ANT ant)
{
	(*super_ant) = ant;
}

//蚂蚁——出生点——随机选择一个城市——返回一个城市编号
int random_city(int num_of_city)
{
	int a = 0;
	srand((unsigned)time(NULL) + rand());
	a = rand() % num_of_city;
	return a;
}

//蚂蚁——轮盘赌选择——每份大小——返回一份小概率
float random_prob(float total_prob)
{
	float a;
	a = 1.0 * (rand() % RAND_MAX) / RAND_MAX * (1.0) * total_prob;
	return a;
}

//蚂蚁——轮盘赌选择——大饼——返回适应度值大小
float R_total_prob(ANT* ant, CITY city[], MAP* map, int num_of_city)
{
	float total_prob_temp = 0.0;
	for (int i = 0; i < num_of_city; i++)
	{
		if (city[i].visited != 1 && i != ant->city_stand)
		{
			total_prob_temp += pow(map->pheromone_graph[ant->city_stand][i], ALPHA) * 
			pow(1.0 / map->distance_graph[(*ant).city_stand][i], BETA);
		}
	}
	return total_prob_temp;
}

//蚂蚁——轮盘赌选择——选择城市——返回一个城市编号
int select_city(ANT *ant, float total_prob, CITY* city, MAP* map)
{
	float temp_prob = random_prob(total_prob);
	float fake_temp_prob = 0.0;
	int i = 0;
	while (fake_temp_prob < temp_prob)
	{
		if (city[i].visited != 1)
		{
			fake_temp_prob += pow(map->pheromone_graph[ant->city_stand][i], ALPHA) * 
			pow(1.0 / map->distance_graph[ant->city_stand][i], BETA);
		}
		i++;
		i = i % map->size_of_map;
	}
	return i;
}

//蚂蚁——计算单只蚂蚁行走距离——修改蚂蚁的路径总距离信息
void sum_distance(ANT* ant, MAP* map)
{
	float total_distance = 0;
	//for (int i = 0; i < 10; i++)
		for (int i = 0; i < city_num - 1; i++)
		{
			total_distance += map->distance_graph[ant->city_visit_path[i]][ant->city_visit_path[i + 1]];
		}

	ant->total_distance = total_distance + map->distance_graph[ant->city_visit_path[9]][ant->city_visit_path[0]];

}

//蚂蚁——城市之间移动一次——修改蚂蚁携带信息以及地图信息
void ant_move(ANT* ant, int next_city, MAP* map)
{
	ant->city_visit_path[ant->city_visited] = next_city;
	ant->total_distance += map->distance_graph[ant->city_stand][next_city];
	ant->city_stand = next_city;
	ant->city_visited++;
}

//蚂蚁——寻路一只蚂蚁的一次回路
void ant_search_path(ANT* ant, int id, MAP* map, int num_of_city, CITY* city)
{
	int flag = 0;
	while (ant->city_visited < num_of_city){
		float fake_total_prob = R_total_prob(ant, city, map, num_of_city);
		int next_city = select_city(ant,fake_total_prob, city, map);

		while (!flag) {
			for (int j = 0; j < num_of_city; j++)
				if ((*ant).city_visit_path[j] == next_city)
					flag = 1;
			if (flag == 1){
				next_city = (next_city + 1) % num_of_city;
				flag = 0;
			}
			else{
				flag = 0;
				break;
			}
		}
		ant_move(ant, next_city, map);
	}
	sum_distance(ant, map);//计算这只蚂蚁的路径
}

//城市——初始化城市——返回一个标准城市单元
void init_city(CITY* city, int num_of_city)
{
	city = (CITY*)malloc(num_of_city * sizeof(CITY));  //生成地图数组
	for (int i = 0; i < num_of_city; i++)
	{
		city[i].visited = 0;                //将其设置为未访问
		city[i].city_lat = -1.0;      //经纬度都是无穷
		city[i].city_lon = -1.0;
	}
}

//地图——初始化地图——返回一张标准地图
void init_map(MAP* map)
{
	map->size_of_map = city_num;
	map->pheromone_graph = (float**)malloc(map->size_of_map * sizeof(float*));
	for (int i = 0; i < map->size_of_map; i++)
	{
		map->pheromone_graph[i] = (float*)malloc(map->size_of_map * sizeof(float));
		for (int j = 0; j < map->size_of_map; j++)
		{
			map->pheromone_graph[i][j] = 1.0;
		}
	}
	map->distance_graph = (float**)malloc(map->size_of_map * sizeof(float*));
	for (int i = 0; i <map->size_of_map; i++)
	{
		map->distance_graph[i] = (float*)malloc(map->size_of_map * sizeof(float));
		for (int j = 0; j < map->size_of_map; j++)
		{
			map->distance_graph[i][j] = 1.0;
		}
	}
}

//地图——计算球面距离——修改地图distance_graph
void sphere_distance(CITY city1, CITY city2, int i, int j, MAP* map)
{
	float en_radian = PI / 180;
	float city1_rad_lat = city1.city_lat * en_radian;
	float city1_rad_lon = city1.city_lon * en_radian;
	float city2_rad_lat = city2.city_lat * en_radian;
	float city2_rad_lon = city2.city_lon * en_radian;
	(*map).distance_graph[i][j] = Equatorial_radius * acos(cos(city1_rad_lat) * cos(city2_rad_lat) * cos(city1_rad_lon - city2_rad_lon) + sin(city1_rad_lat) * sin(city2_rad_lat));
}

//地图——every 10 ants更新地图信息素分布——修改地图中的信息素成员
void map_updata_pheromone(MAP* map, int num_of_city, ANT ant[])
{
	float** temp_pheromone;
	temp_pheromone = (float**)malloc(num_of_city * sizeof(float*));
	for (int i = 0; i < num_of_city; i++){
		temp_pheromone[i] = (float*)malloc(num_of_city * sizeof(float));
		for (int j = 0; j < num_of_city; j++)
		{
			temp_pheromone[i][j] = 0.0;
		}
	}
	for (int i = 0; i < ant_num; i++){
		for (int j = 1; j < city_num - 1; j++){
			int start, end;
			start = ant[i].city_visit_path[j - 1];
			end = ant[i].city_visit_path[j];
			temp_pheromone[start][end] += (Q / ant[i].total_distance) * map->distance_graph[start][end];
			temp_pheromone[end][start] = temp_pheromone[start][end];
			// 2 1 0 4 5 6 9 8 7
		}
		temp_pheromone[ant[i].city_visit_path[0]][ant[i].city_visit_path[9]] +=
		(Q / ant[i].total_distance) * map->distance_graph[ant[i].city_visit_path[0]][ant[i].city_visit_path[9]];
		temp_pheromone[ant[i].city_visit_path[9]][ant[i].city_visit_path[0]] = 
		temp_pheromone[9][ant[i].city_visit_path[9]];
	}
	for (int i = 0; i < num_of_city; i++){
		for (int j = 0; j < num_of_city; j++){
			(map->pheromone_graph)[i][j] = map->pheromone_graph[i][j] * RHO + temp_pheromone[i][j];
		}
	}
}

//TSP——初始化一个旅行商人问题——
void init_TSP(ANT ant[], ANT* super_ant, CITY city[], MAP* map, int num_of_city, int num_of_ant)
{
	init_map(map);
	//对蚂蚁进行id的初始化 方便后来访问
	for (int i = 0; i < num_of_ant; i++){
		ant[i].ID = i;
	}
	super_ant = ant;//super_ant是提供最佳路线的蚂蚁，每次其他蚂蚁有了短的路径都要和超级蚂蚁比较
	super_ant->total_distance = -1.0;
	//填写城市名称
	char city_name[10][20] = {
		"Changchun", "Harbin", "Peking","Shanghai","Chongqing",
		"Zhengzhou","Nanjing","Chengdu","Wuhan","Taibei"
	};
	//每个市对应的经纬度
	float city_lat[10] = { 125.326800, 126.662850, 116.407170, 121.473700, 106.550730, 
	113.753220, 118.762950, 104.075720, 114.342340, 121.520076 };
	float city_lon[10] = { 43.896160,  45.742080,  39.904690,  31.230370,  29.564710,  
	34.765710,  32.060710,  30.650890,  30.545390,  25.030724 };
	for (int i = 0; i < num_of_city; i++){
		city[i].name = city_name[i];
		city[i].city_lat = city_lat[i];
		city[i].city_lon = city_lon[i];
	}
	for (int i = 0; i < num_of_city; i++){
		for (int j = 0; j < num_of_city; j++){
			sphere_distance(city[i], city[j], i, j, map);//计算球面距离
		}
	}
}

//旅行商人——寻路——一次
void search_1_TSP(ANT ant[], ANT* super_ant, CITY* city, MAP* map, int num_of_city, int num_of_ant)
{
	for (int i = 0; i < num_of_ant; i++)
	{
		init_ant(&ant[i], i, city, city_num);
		ant_search_path(ant + i, i, map, num_of_city, city);
		if (ant[i].total_distance < (*super_ant).total_distance)
		{
			*super_ant = ant[i];
		}
	}
	map_updata_pheromone(map, num_of_city, ant);
	iter += 1;
	printf("迭代次数：%d\n最佳路径总距离：%f\n最佳路径：\n", iter, (*super_ant).total_distance);
	for (int i = 0; i < city_num; i++)
		printf("%s -> ", city[super_ant->city_visit_path[i]].name);
	printf("%s\n", city[super_ant->city_visit_path[0]].name);
}

//旅行商人——寻路——times
void search_n_TSP(ANT ant[], ANT* super_ant, CITY* city, MAP* map, int num_of_city, int num_of_ant, int times_of_search){
	//每迭代一次更新一次信息素矩阵
	for (int j = 0; j < times_of_search; j++){
		//每十个蚂蚁获得一个蚂蚁路径最小值，与超级蚂蚁比较
		for (int i = 0; i < num_of_ant; i++){
			init_ant(&ant[i], i, city, city_num);
			ant_search_path(ant + i, i, map, num_of_city, city);
			if (0 == i && 0 == j)
				*super_ant = ant[i];
			if (ant[i].total_distance < (*super_ant).total_distance){
				*super_ant = ant[i];
			}
		}
		map_updata_pheromone(map, num_of_city, ant);//更新信息素举证
		iter += 1;
		printf("迭代次数：%d\n最佳路径总距离：%f\n最佳路径：\n", iter, (*super_ant).total_distance);
		for (int i = 0; i < city_num; i++)
			printf("%d-> ", super_ant->city_visit_path[i]);
		printf("%d\n", super_ant->city_visit_path[0]);
	}
}

//旅行商人——解决一个寻路问题
void solution_TSP()
{
	ANT ant[ant_num];
	ANT super_ant;
	CITY city[city_num];
	MAP map = { 0};
	
	init_TSP(ant, &super_ant, city, &map, city_num, ant_num);

	printf("输入s开始进入搜索模式\n输入q退出程序");
	char flag = 's';
	scanf("%c", &flag);
	getchar();
	while ('q' != flag)
	{
		int times = 0;
		printf("请输入迭代次数：\n");
		scanf("%d", &times);
		getchar();
		search_n_TSP(ant, &super_ant, city, &map, city_num, ant_num, times);
		while ('n' != flag)
		{
			printf("你需要继续迭代吗？（y/n）"); 
			scanf("%c", &flag);
			if (flag == 'n') break;
			printf("请输入迭代次数：");
			scanf("%d", &times);
			for (int i = 0; i < times; i++)
				search_1_TSP(ant, &super_ant, city, &map, city_num, ant_num);
		}
		printf("是否退出程序？（q?）");
		scanf("%c", &flag);
		getchar();

	}
	printf("the ant_system will exit......");
	exit(0);
}

int main()
{
	solution_TSP();
}