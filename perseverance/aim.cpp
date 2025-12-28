#include "aim.h"
#include "imgui/imgui.h"

void MoveMouseRelative(int dx, int dy)
{
    INPUT input = { 0 };
    input.type = INPUT_MOUSE;
    input.mi.dx = dx;
    input.mi.dy = dy;
    input.mi.dwFlags = MOUSEEVENTF_MOVE;

    SendInput(1, &input, sizeof(INPUT));
}


void aim_loop(std::shared_ptr<Cache> c)
{
	if (!c) return;
    if (!settings::aim) return;

    const PlayerEntity* closest_player = nullptr;
    float closest_dist = FLT_MAX;

    if (settings::show_fov)
        ImGui::GetBackgroundDrawList()->AddCircle(ImVec2(c->Width / 2, c->Height / 2), settings::fov, ImColor(255, 255, 255), 16);


    const LocalPlayer& local = c->get_local();
    if (!local.is_valid())
        return;

    const auto& players = c->get_players();
    for (const auto& player : players)
    {
        if (!player.is_valid() || !player.is_alive() || !player.is_enemy(local.get_team()))
            continue;
            
        vec3 head = player.get_bone(player.head);
        vec2 head2d;

        if (!World2Screen(head, head2d, c->viewMatrix.matrix, c->Width, c->Height))
            continue;

        double dx = head2d.x - c->Width / 2;
        double dy = head2d.y - c->Height / 2;
        float dist = sqrtf(static_cast<float>(dx * dx + dy * dy));

        if (dist <= settings::fov && dist < closest_dist)
        {
            closest_dist = dist;
            closest_player = &player;
        }
    }

    if (!closest_player) return;

    vec3 aimpard3d = closest_player->get_bone(closest_player->head);
    vec2 aimpart;
    if (!World2Screen(aimpard3d, aimpart, c->viewMatrix.matrix, c->Width, c->Height))
        return;

    if (GetAsyncKeyState(settings::keybind))
    {
        float centerX = c->Width / 2.0f;
        float centerY = c->Height / 2.0f;

        float dx = aimpart.x - centerX;
        float dy = aimpart.y - centerY;

        const float deadzone = 0.00f;

        if (fabsf(dx) < deadzone && fabsf(dy) < deadzone)
            return;

        dx /= settings::smoothing;
        dy /= settings::smoothing;

        MoveMouseRelative(dx, dy);
    }
}