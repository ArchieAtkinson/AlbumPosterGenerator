#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include "../external/raylib/src/raylib.h"
#include "../external/raylib/src/raymath.h"

#include "../external/argparse/include/argparse/argparse.hpp"

#include "../inc/album_info.h"
#include "../inc/display.h"
#include "../inc/root_dir.h"

int screenWidth = 1080;
int screenHeight = 1920;

const int posterWidth = 3508;
const int posterHeight = 4961;

const int border = int(posterWidth * 0.05);
const int line_thickness = static_cast<int>(static_cast<float>(border) * 0.05f);
const int cover_name_gap = border * 1.1f;
const int std_num_tracks_per_col = 10;

RenderTexture viewport;
Texture2D create_album_cover_texture(std::string filepath)
{
    const char *filepath_c = filepath.c_str();
    auto img = LoadImage(filepath_c);
    int img_size = posterWidth - (border * 2);
    ImageResize(&img, img_size, img_size);
    auto texture = LoadTextureFromImage(img);
    UnloadImage(img);
    return texture;
}

struct Columns{
    std::string left;
    std::string right;
    int count;
};

Columns create_track_col(std::vector<std::string>& track_list){
    int left_count, right_count;
    Columns columns;
    if(track_list.size() <= std_num_tracks_per_col){
        left_count = track_list.size();
        right_count = 0;
        columns.count = std_num_tracks_per_col;
    }
    else if(track_list.size() > std_num_tracks_per_col && track_list.size() < std_num_tracks_per_col * 2){
        left_count = std_num_tracks_per_col;
        right_count = track_list.size() - std_num_tracks_per_col;
        columns.count = std_num_tracks_per_col;        
    }
    else {
        left_count = static_cast<int> (ceil(track_list.size()/2.0f));
        right_count = static_cast<int> (floor(track_list.size()/2.0f));
        columns.count = left_count;           
    }

    for (auto i = 0; i < left_count; i++){
        columns.left += track_list[i] + "\n ";
    }
    for (auto i = left_count; i < right_count + left_count; i++){
        columns.right += track_list[i] + "\n ";
    }
    return columns;
}

std::string space2underscore(std::string text)
{
    std::replace(text.begin(), text.end(), ' ', '_');
    return text;
}

int main(int argc, char const *argv[])
{
    // Initialization
    //--------------------------------------------------------------------------------------

    argparse::ArgumentParser program("APG");
    program.add_argument("artwork").default_value(APG::ROOT_DIR + "/example_data/raceforspace.png");
    program.add_argument("info").default_value(APG::ROOT_DIR + "/example_data/raceforspace.txt");

    try
    {
        program.parse_args(argc, argv);
    }
    catch (const std::runtime_error &err)
    {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        std::exit(1);
    }

    auto cover_art_path = program.get<std::string>("artwork");
    auto info_file_path = program.get<std::string>("info");

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "APG");
    MaximizeWindow();
    SetTargetFPS(60);

    viewport = LoadRenderTexture(posterWidth, posterHeight);

    std::string fontpath_black = APG::ROOT_DIR + "/resources/Urbanist-Black.ttf";
    std::string fontpath_bold = APG::ROOT_DIR + "/resources/Urbanist-Bold.ttf";
    Font font_black = LoadFontEx(fontpath_black.c_str(), 256, 0, 0);
    Font font_bold = LoadFontEx(fontpath_bold.c_str(), 256, 0, 0);

    auto cover_art_tex = create_album_cover_texture(cover_art_path);
    auto album_info = AlbumInfo(info_file_path);

    TextLine album_name(album_info.album_name, font_black, posterWidth * 0.075f);
    album_name.set_reference_point({border, static_cast<float>(border) + cover_art_tex.height});
    album_name.set_offset({0, album_name.size * 0.1f});
    
    TextLine artist_name(album_info.artist_name, font_bold, posterWidth * 0.035f);
    artist_name.set_reference_point(album_name.get_position());
    artist_name.set_offset({0, album_name.size});

    Vector2 line_start{border, artist_name.get_position().y + artist_name.size * 1.1f};
    Vector2 line_end{posterWidth - border, line_start.y};

    TextLine album_date(album_info.album_date, font_bold, posterWidth * 0.025f);
    album_date.set_reference_point({posterWidth - border, line_end.y});
    album_date.set_offset({-album_date.get_length(), -(artist_name.size * 0.1f + album_date.size)});
    
    TextLine album_duration(album_info.album_duration, font_bold, posterWidth * 0.025f);
    album_duration.set_reference_point(album_date.get_reference_point());
    album_duration.set_offset({-album_duration.get_length(), artist_name.size * 0.1f});
    
    int num_tracks_per_col = std_num_tracks_per_col;

    if(album_info.track_names.size() > std_num_tracks_per_col * 2){
        num_tracks_per_col = static_cast<int>(ceil(album_info.track_names.size() / 2));
    }

    Columns columns = create_track_col(album_info.track_names);

    float space_for_track_list = (posterHeight - border) - (line_start.y + artist_name.size * 0.1f);
    int track_font_size = static_cast<int>(round(space_for_track_list / static_cast<double>(columns.count)));

    float line_length = line_end.x - line_start.x;
    TextBox left_track_TB(columns.left, font_bold, track_font_size, {line_length, space_for_track_list});
    left_track_TB.set_reference_point({line_start});
    left_track_TB.set_offset({0, artist_name.size * 0.1f});
    
    TextBox right_track_TB(columns.right, font_bold, track_font_size, {line_length, space_for_track_list});
    right_track_TB.set_reference_point({line_start.x + (line_start.x + line_end.x)/2, line_start.y});
    right_track_TB.set_offset({0, artist_name.size * 0.1f});
        
    while (!WindowShouldClose()) // Detect window close button or ESC ey
    {
        BeginTextureMode(viewport);

        DrawRectangle(0, 0, posterWidth, posterHeight, RAYWHITE);
        DrawTexture(cover_art_tex, border, border, WHITE);
        DrawLineEx(line_start, line_end, line_thickness, BLACK);
        album_name.draw();
        artist_name.draw();
        album_date.draw();
        album_duration.draw();
        left_track_TB.draw();
        right_track_TB.draw();

        EndTextureMode();

        BeginDrawing();

        ClearBackground(RAYWHITE);

        float display_height = static_cast<float>(GetScreenHeight());
        float display_width = static_cast<float>(GetScreenHeight()) / 1.414f;

        DrawTexturePro(viewport.texture, {0, 0, posterWidth, -posterHeight},
                       {(GetScreenWidth() - display_width) / 2, 0, display_width, display_height}, {0, 0}, 0.0f, WHITE);

        int line_thickness = GetScreenHeight() * 0.002f > 1 ? static_cast<int> (round( GetScreenHeight() * 0.002f)) : 1;

        DrawRectangleLinesEx({(GetScreenWidth() - display_width) / 2, 0, display_width, display_height}, line_thickness, BLACK);

        EndDrawing();

        //----------------------------------------------------------------------------------
    }

#ifndef DEBUG
    Image viewport_img = LoadImageFromTexture(viewport.texture);
    ImageFlipVertical(&viewport_img);
    
    const char *output_file_name =
        TextFormat("%s-%s.png", space2underscore(album_info.album_name).c_str(), space2underscore(album_info.artist_name).c_str());
    ExportImage(viewport_img, output_file_name);
    UnloadImage(viewport_img);
#endif

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}