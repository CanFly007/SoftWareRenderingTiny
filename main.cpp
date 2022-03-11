#include <vector>
#include <cmath>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
Model* model = NULL;
const int width = 200;
const int height = 200;

void line(Vec2i p0, Vec2i p1, TGAImage& image, TGAColor color)
{
    bool steep = false;
    if (std::abs(p0.x - p1.x) < std::abs(p0.y - p1.y))//x轴的跨度小于Y轴，就比较陡，选用Y轴递增
    {
        std::swap(p0.x, p0.y);
        std::swap(p1.x, p1.y);//x值就变成Y值了，X的跨度就变为相对较大的
        steep = true;
    }
    if (p0.x > p1.x)//起始点高于结束点 调个
    {
        std::swap(p0, p1);
    }
    for (int x = p0.x; x <= p1.x; x++)
    {
        float t = (x - p0.x) / (float)(p1.x - p0.x);
        int y = p0.y + t * (p1.y - p0.y);
        if (steep) 
        {
            image.set(y, x, color);
        }
        else 
        {
            image.set(x, y, color);
        }
    }
}

Vec3f barycentric(Vec2i* pts, Vec2i P) 
{
    //不管三角形P1 P2 P3顺序按照顺时针还是逆时针（可画图）
    //分母三角形面积都可以按照P1P2叉乘P1P3来算
    //分子三角形也是PP1 PP2 PP3顺序
    //P1P2 P1P3
    float triangleArea = 0.5 * ((pts[1] - pts[0]) ^ (pts[2] - pts[0]));
    //PP2 PP3
    float uTriangle = 0.5 * ((pts[1] - P) ^ (pts[2] - P));
    //PP3 PP1
    float vTriangle = 0.5 * ((pts[2] - P) ^ (pts[0] - P));
    //PP1 PP2
    float wTriangle = 0.5 * ((pts[0] - P) ^ (pts[1] - P));

    float u = uTriangle / triangleArea;
    float v = vTriangle / triangleArea;
    float w = wTriangle / triangleArea;
    if (u < 0 || v < 0 || w < 0)
        return Vec3f(-1, -1, -1);
    return Vec3f(u, v, w);
}

void triangle(Vec2i* pts, TGAImage& image, TGAColor color) 
{
    Vec2i bboxmin(image.get_width() - 1, image.get_height() - 1);
    Vec2i bboxmax(0, 0);
    Vec2i clamp(image.get_width() - 1, image.get_height() - 1);
    //循环三个点，找到pts这个三角形的最小最大值，包围盒
    for (int i = 0; i < 3; i++) 
    {
        bboxmin.x = std::max(0, std::min(bboxmin.x, pts[i].x));//100
        bboxmin.y = std::max(0, std::min(bboxmin.y, pts[i].y));//30

        bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, pts[i].x));//100
        bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, pts[i].y));//
    }
    Vec2i P;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) 
    {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) 
        {
            Vec3f bc_screen = barycentric(pts, P);
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;
            //image.set(P.x, P.y, color);
            image.set(P.x, P.y, TGAColor(bc_screen.x * 255, bc_screen.y * 255, bc_screen.z * 255, 255));
        }
    }
}


void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage& image, TGAColor color)
{
    // sort the vertices, t0, t1, t2 lower−to−upper (bubblesort yay!) 
    if (t0.y > t1.y)std::swap(t0, t1);
    if (t0.y > t2.y) std::swap(t0, t2);
    if (t1.y > t2.y) std::swap(t1, t2);
    int total_height = t2.y - t0.y;
    for (int y = t0.y; y < t1.y; y++)
    {
        int segment_height = t1.y - t0.y + 1;
        float alpha = (float)(y - t0.y) / total_height;
        float beta = (float)(y - t0.y) / segment_height;// be careful with divisions by zero 
        Vec2i A = t0 + (t2 - t0) * alpha;
        Vec2i B = t0 + (t1 - t0) * beta;
        if (A.x > B.x) std::swap(A, B);
        for (int j = A.x; j < B.x; j++)//按x轴方向填充内部
        {
            image.set(j, y, color);
        }
    }
    for (int y = t1.y; y <= t2.y; y++)//三角形上半部分
    {
        int segment_height = t2.y - t1.y + 1;
        float alpha = (float)(y - t0.y) / total_height;
        float beta = (float)(y - t1.y) / segment_height;// be careful with divisions by zero
        Vec2i A = t0 + (t2 - t0) * alpha;
        Vec2i B = t1 + (t2 - t1) * beta;
        if (A.x > B.x) std::swap(A, B);
        for (int j = A.x; j < B.x; j++)//按x轴方向填充内部
        {
            image.set(j, y, color);
        }
    }
}

int main(int argc, char** argv) 
{
    TGAImage frame(200, 200, TGAImage::RGB);
    Vec2i pts[3] = { Vec2i(10,10), Vec2i(100, 30), Vec2i(190, 160) };
    triangle(pts, frame, TGAColor(255, 0, 0, 255));
    frame.flip_vertically(); // to place the origin in the bottom left corner of the image 
    frame.write_tga_file("output.tga");
    return 0;

    if (2 == argc)
    {
        model = new Model(argv[1]);
    }
    else
    {
        model = new Model("obj/african_head.obj");
    }
    TGAImage image(width, height, TGAImage::RGB);

    Vec2i t0[3] = { Vec2i(10,70),Vec2i(50, 160),  Vec2i(70, 80) };
    Vec2i t1[3] = { Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180) };
    Vec2i t2[3] = { Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180) };

    triangle(t0[0], t0[1], t0[2], image, red);
    triangle(t1[0], t1[1], t1[2], image, white);
    triangle(t2[0], t2[1], t2[2], image, green);
    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");
    return 0;
}