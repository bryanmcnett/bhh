#include "stdio.h"
#include <vector>
#include <time.h>
#include <math.h>
#include <algorithm>

struct Clock
{
  const clock_t m_start;
  Clock() : m_start(clock())
  {
  }
  float seconds() const
  {
    const clock_t end = clock();
    const float seconds = ((float)(end - m_start)) / CLOCKS_PER_SEC;
    return seconds;
  }
};

struct float3
{
  float x,y,z;
};
  
float3 operator+(const float3 a, const float3 b)
{
  float3 c = {a.x+b.x, a.y+b.y, a.z+b.z};
  return c;
}

float dot(const float3 a, const float3 b)
{
  return a.x*b.x + a.y*b.y + a.z*b.z;
}

float length(const float3 a)
{
  return sqrtf(dot(a,a));
}

float3 min(const float3 a, const float3 b)
{
  float3 c = {std::min(a.x,b.x), std::min(a.y,b.y), std::min(a.z,b.z)};
  return c;
}

float3 max(const float3 a, const float3 b)
{
  float3 c = {std::max(a.x,b.x), std::max(a.y,b.y), std::max(a.z,b.z)};
  return c;
}

struct AABB
{
  float3 m_min;
  float3 m_max;
};

int intersects(const AABB a, const AABB b)
{
  if(a.m_min.x <= b.m_max.x
  && a.m_max.x >= b.m_min.x
  && a.m_min.y <= b.m_max.y
  && a.m_max.y >= b.m_min.y
  && a.m_min.z <= b.m_max.z
  && a.m_max.z >= b.m_min.z)
    return 1;
  return 0;    
}

float random(float lo, float hi)
{
  const int grain = 10000;
  const float t = (rand() % grain) * 1.f/(grain-1);
  return lo + (hi - lo) * t;
}

struct Mesh
{
  std::vector<float3> m_point;
  void Generate(int points, float radius)
  {
    m_point.resize(points);
    for(int p = 0; p < points; ++p)
    {
      do
      {
        m_point[p].x = random(-radius, radius);
        m_point[p].y = random(-radius, radius);
        m_point[p].z = random(-radius, radius);
      } while(length(m_point[p]) > radius);
    }
  }
};

struct Object
{
  Mesh *m_mesh;
  float3 m_position;
  void CalculateAABB(AABB* aabb) const
  {
    const float3 xyz = m_position + m_mesh->m_point[0];
    aabb->m_min = aabb->m_max = xyz;
    for(int p = 1; p < m_mesh->m_point.size(); ++p)
    {
      const float3 xyz = m_position + m_mesh->m_point[p];
      aabb->m_min = min(aabb->m_min, xyz);
      aabb->m_max = max(aabb->m_max, xyz);
    }
  }
};

struct bhh_compare
{
  int m_direction;
  bhh_compare(int direction) : m_direction(direction) {}
  bool operator ()(const AABB& a, const AABB& b) const
  {
    switch(m_direction)
    {
    case 0: return a.m_min.x < b.m_min.x;
    case 1: return a.m_min.y < b.m_min.y;
    case 2: return a.m_min.z < b.m_min.z;
    case 3: return -(a.m_max.x + a.m_max.y + a.m_max.z) < -(b.m_max.x + b.m_max.y + b.m_max.z);
    }
    return false;
  }
};

struct bhh_reject
{
  int m_direction;
  bhh_reject(int direction) : m_direction(direction) {}
  bool operator()(const AABB& aabb, const AABB& query) const
  {
    switch(m_direction)
    {
    case 0: return query.m_max.x < aabb.m_min.x; break;
    case 1: return query.m_max.y < aabb.m_min.y; break;
    case 2: return query.m_max.z < aabb.m_min.z; break;
    case 3: return -(query.m_min.x + query.m_min.y + query.m_min.z)
                 < -(aabb.m_max.x  + aabb.m_max.y  + aabb.m_max.z );      
    }
    return false;
  }
};

void bhh_sort(AABB* begin, AABB* end, int direction = 0)
{
  if(end - begin < 2)
    return;
  AABB* median = begin + (end - begin) / 2;
  std::nth_element(begin, median, end, bhh_compare(direction));
  bhh_sort(begin, median, (direction + 1) & 3);
  bhh_sort(median+1, end, (direction + 1) & 3);
}

int bhh_search(const AABB* begin, const AABB* end, const AABB query, int direction = 0)
{
  switch(end - begin)
  {
    case 1: return intersects(*begin, query);
    case 0: return 0;
  }
  const AABB* median = begin + (end - begin) / 2;
  const int intersections = bhh_search(begin, median, query, (direction + 1) & 3);
  if(bhh_reject(direction)(*median, query))
    return intersections;
  return intersections + intersects(*median, query) + bhh_search(median+1, end, query, (direction + 1) & 3);
}

int main(int argc, char* argv[])
{
  Mesh mesh;
  mesh.Generate(100, 1.0f);

  const int kTests = 100;
  
  const int kObjects = 1000000;
  std::vector<Object> objects(kObjects);
  for(int o = 0; o < kObjects; ++o)
  {
    objects[o].m_mesh = &mesh;
    objects[o].m_position.x = random(-50.f, 50.f);
    objects[o].m_position.y = random(-50.f, 50.f);
    objects[o].m_position.z = random(-50.f, 50.f);
  }
  
  std::vector<AABB> unsorted(kObjects);
  for(int a = 0; a < kObjects; ++a)
    objects[a].CalculateAABB(&unsorted[a]);

  std::vector<AABB> sorted = unsorted;
  bhh_sort(&sorted[0], &sorted[0]+sorted.size());

  {
    const Clock clock;
    int intersections = 0;
    for(int test = 0; test < kTests; ++test)
    {
      const AABB query = unsorted[test];
      for(int t = 0; t < kObjects; ++t)
      {
        const AABB target = unsorted[t];
	if(intersects(target, query))
  	  ++intersections;
      }
    }
    const float seconds = clock.seconds();
    
    printf("unsorted AABB array reported %d intersections in %f seconds\n", intersections, seconds);
  }
  
  {
    const Clock clock;
    int intersections = 0;
    for(int test = 0; test < kTests; ++test)
    {      
      const AABB query = unsorted[test];
      intersections += bhh_search(&sorted[0], &sorted[0]+sorted.size(), query);
    }
    const float seconds = clock.seconds();
    
    printf("sorted AABB array reported %d intersections in %f seconds\n", intersections, seconds);
  }
  return 0;
}
