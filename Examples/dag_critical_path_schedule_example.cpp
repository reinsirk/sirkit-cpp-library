#include "../Graph/DAG_Critical_Path_Schedule.cpp"

#include <cassert>
#include <vector>

int main()
{
    const std::vector<sirkit::DagScheduleEdge> edges{{0, 1, 3}, {1, 2, 7}, {0, 2, 8}};
    const auto schedule = sirkit::dag_critical_path_schedule(3, edges, 0, 2);
    assert(schedule && schedule->duration == 10);
    assert(schedule->earliest[1] == schedule->latest[1]);
    assert(schedule->edge_slack[2] == 2);
}
