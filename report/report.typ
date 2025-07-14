#import "conf.typ": *
#show: conf

#title([真实感渲染大作业  实验报告])
#author([Sayon])
#author([June 19, 2025])
#v(10pt)


= 实现功能

== Whitted-Style 光线追踪
#v(3pt)
支持仅考虑一次漫反射、理想反射、理想折射，以及阴影效果的 Whitted-Style 光线追踪。

== 路径追踪
#v(3pt)
支持面光源、无穷递归、RR 终止、BRDF 着色、NEE 的路径追踪。实现了漫反射（Lambert）、反射折射（理想反射、理想折射、菲涅尔效应的玻璃）、光滑面（Modified Phong、Cook-Torrance）等多种 BRDF 材质。

支持按照 BRDF 重要性采样、支持 BRDF 采样与 NEE 相结合的 MIS 采样。

支持 BVH 加速。

支持法向插值、法向贴图、纹理贴图、景深、抗锯齿、OpenMP 并行。

= 原理简述

== Whitted-Style 光线追踪
#v(3pt)
对于成像平面的每个像素点，投射一束光线，碰到漫反射表面则与所有光源计算着色（Phong模型）并终止，期间检查是否被遮挡而相应乘上遮挡系数；碰到理想反射或理想折射表面则根据物理方程计算反射 / 折射光线；直到达到设定的最大深度。

#subpar.grid(
  columns: (1fr, 1fr),
  figure(
    image("fig/whitted_three_balls_one_glass.png"),
    caption: "Whitted-Style 光线追踪"
  ), <fig:whitted>,
  figure(
    image("fig/mc_CMP_whitted_three_balls_one_glass_128spp_116s_nee.png"), 
    caption: "路径追踪, NEE, 128spp"
  ), <fig:mc>,
  caption: "Whitted-Style 光线追踪与路径追踪的对比",
)

== 路径追踪：BRDF 着色与 NEE 采样

=== 渲染方程与 BRDF 着色

渲染的本质是求解渲染方程
$ L_o (p, omega_o) = L_e (p, omega_o) + integral_(HH^+) L_i (p, omega_i) f(p, omega_i, omega_o) (omega_i, omega_n) dif omega_i $
其中 $f(dot.c)$ 为 BRDF 函数。对于理想反射、理想折射等表面，BRDF 函数具有 Dirac delta 分布，并可直接计算光线的下一方向，即上式积分事实退化；对于一般的表面，BRDF 函数非退化，则需要用蒙特卡洛方法计算积分。材料各不相同的反射特性对应不同的 BRDF 函数，从而使渲染结果具有符合物理世界的不同着色。

图@fig:mc 是用路径追踪渲染的与图@fig:whitted (Whitted-Style 光线追踪) 相同场景的图片（除了将点光源换成面光源）。可以看出，路径追踪的渲染结果更符合人的物理认知，颜色过渡更平滑自然（如阴影）。另一方面，由于路径追踪是用蒙特卡洛解渲染方程，这一估计方差在渲染图中体现为噪点。噪点的大小与采样数成反比，图中128spp 已经有了较好的收敛效果。

=== NEE 采样

对于光源数量较少、面积较小的场景，在上半球面采样一个点的入射方向时，无论是随机采样还是根据 BRDF 形式设计的重要性采样（详见下节），采样到光源的概率较低，使渲染结果收敛较慢。NEE 采样则是在采样入射方向的同时，直接从光源采样，从而有效地加速了收敛。

在实现 NEE 时，*有两点需要注意*：
- 一是注意概率测度的变换，即在光源上采样面积微元 $dif upright(A)$ 后，通过 $dif omega = cos(theta_"light")/d^2 dif upright(A)$ 转化为上半球面的立体角微元，其中 $theta_"light"$ 为光源表面法向与光线方向的夹角，$d$ 为光源表面到交点的距离；
- 二是注意不要重复计算光源贡献，即当光线打到光源上时，若上一交点材质的 BRDF 非退化，则上一交点的 NEE 已经计算光源贡献，此时应不再考虑光源自身辐射。

#subpar.grid(
  columns: (1fr, 1fr),
  figure(
    image("fig/cornell_box_128spp_81s_cosine_naive.png"), 
    caption: "Naive, 128spp, 81s"
  ),
  figure(
    image("fig/cornell_box_16spp_17s_cosine_nee.png"), 
    caption: "NEE, 16spp, 17s"
  ),
  caption: "直接采样与 NEE 采样的效果对比",
  label: <fig:nee_cmp>
)

如图@fig:nee_cmp 所示，虽然 NEE 额外的对光源采样稍有增加每 spp 的渲染时间，但其收敛速度大幅提高。可以看见，16spp 的 NEE 采样效果已经大幅超过了 128spp 的 Naive 采样。特别是对于小光源的场景，Naive 的路径追踪很难命中光源，绝大部分光线都是低效甚至无效投射；而 NEE 通过显式的采样光源，有效提高了采样效率。


== 蒙特卡洛积分与 BRDF 重要性采样
#v(3pt)
蒙特卡洛积分 $display(EE_(X~p(X)) [f(X)/p(X)] = integral f(x) dif x)$ 是一个无偏估计，以 $display(hat(I) = 1/N sum_(i=1)^N f(x_i)/p(x_i))$ 作为积分的近似值。因此，我们寻求逼近 $f(x)$ 足够好的概率密度函数 $p(x)$，使得 $display("Var"(hat(I)) = 1/N "Var"[f(X)/p(X)])$ 尽可能小。

对于渲染方程中的积分 $display(integral_(HH^+) L_i (p, omega_i) f(p, omega_i, omega_o) (omega_i, omega_n) dif omega_i)$，我们无从得知 $L_i (p, omega_i)$，不妨视之为一个常数；因此，我们针对 $f(p, omega_i, omega_o) (omega_i, omega_n)$ 设计重要性采样的概率密度函数。

=== Lambert 材质：cosine-weighted 采样
#v(3pt)
对于 Lambert 材质，BRDF 函数为一定值，因此设计 $p(omega) prop (omega, omega_n)$。在以 $omega_n$ 为轴的球坐标下，$p(theta, phi) = sin theta dot.c p(omega) prop sin theta cos theta$，从中得知 $theta, phi$ 独立（因此可以分别采样），进一步计算有
$ p(phi)=1/(2pi), F(phi)=phi/(2pi); quad p(theta)=2 sin theta cos theta, F(theta)=1-cos^2 theta $
因此，我们可以通过 $theta = arccos(sqrt(1-u_1)), phi = 2pi u_2$ 采样得到入射方向，其中 $u_1, u_2 ~ U[0, 1]$。

=== Modified Phong 材质
#v(3pt)
Modified Phong 的 BRDF 高光部分 $prop (omega, omega_r)^alpha$，其中 $omega_r$ 为出射方向 $omega_o$ 关于 $omega_n$ 的对称方向；几何项 $(omega, omega_n)$。但我们难以得到 $p(omega) prop (omega, omega_r)^alpha dot.c (omega, omega_n)$ 的解析形式，因此有两种自然的处理方式：

- 一是舍弃 $(omega, omega_n)$，设计 $p(omega) prop (omega, omega_r)^alpha$。在以 $omega_r$ 为轴的球坐标下，接下来的推导与 Lambert 材质类似，最终有 $F(theta) = 1 - cos^(alpha+1)theta$，采样 $theta = arccos (1-u_1)^(1/(alpha+1)), phi = 2pi u_2$；
- 二是认为 $(omega, omega_n) approx (omega, omega_r)$，采样 $theta = arccos (1-u_1)^(1/(alpha+2)), phi = 2pi u_2$。

实践中，两种采样方法没有明显的差异。

我们以 $P = k_s / (k_s + k_d)$ 的概率用上述高光采样，以 $1-P$ 的概率用 cosine-weighted 采样。

=== Cook-Torrance 材质
#v(3pt)
Cook-Torrance 的 BRDF 函数 $f = k_d 1/pi + k_s (F(omega_o, omega_n)G(omega_i, omega_o, omega_n)D(omega_h, omega_n))/(4(omega_i, omega_n)(omega_o, omega_n))$。其中 $omega_h = (omega_i+omega_o)/norm(omega_i+omega_o)$ 为半程向量；$F$ 为 Fresnel 方程，实现中采用 Fresnel-Schlick 近似；$G$ 为几何项，实现中采用 Smith 近似以及 Schlick-GGX 方程；$D$ 为法线分布函数，实现中采用 GGX 分布
$ D_"GGX" (omega_h, omega_n) = alpha^2/(pi ((omega_h, omega_n)^2 (alpha^2-1)+1)^2) $
我们设计 $p(omega) prop D_"GGX" (omega_h, omega_n) (omega_i, omega_n)$，这和 Modified Phong 一样无法直接处理，但第一种近似方式还是求不出解析形式，因此我们采用第二种近似方式 $p(omega) approx D_"GGX" (omega_h, omega_n)(omega_h, omega_n)$，注意法向分布函数需要满足的性质保证上式积分恰好为1。最后得到 $omega_h$ 在以 $omega_n$ 为轴的球坐标下的采样 $theta = arccos sqrt((1-u_1)/(u_1 (alpha^2-1)+1))), phi = 2pi u_2$。

*需要特别注意*，我们采样的是半程向量 $omega_h$，而不是入射方向 $omega_i$。因此最后我们不仅需要用 $omega_h$ 计算 $omega_i$，还需要变换概率密度函数 $p(omega_i) = p(omega_h) / (4 (omega_o, omega_h))$，其中 $1/(4 (omega_o, omega_h))$ 为变换的 Jacobian，并不平凡！

#figure(
  image("fig/cornell_box_balls_128spp_cmp.png"),
  caption: "Cook-Torrance 材质不同采样方式对比 (128spp)。左：uniform 采样，右：Cook-Torrance 采样"
)

== BRDF 采样与 NEE 相结合的 MIS 采样
#v(3pt)
BRDF 采样能较好处理大光源、反射集中的材质，而 NEE 能较好处理小而多的光源、远离光源的粗糙面。MIS 采样的思想是，对于两种采样方式，我们分别计算积分的估计值，然后按照两种采样方式的概率密度函数的比值加权平均，从而得到一个更好的估计值。

我们采用原论文提出的 Balanced Heuristic：对第 $i$ 种采样策略（这里 $i=1,2$），使加权后的估计值 $w_i (x) f(x)/(p_i (x))$ 与 $i$ 无关，即让所有采样策略贡献“相等”。具体地，即取 $w_i = p_i / (p_1+p_2)$。

- 对于打到光源、上一交点 BRDF 非退化的光线，已有上一步的 BRDF 采样概率 $p_1$。我们计算上一交点若用 NEE 采样到此点的概率 $p_2$（*同样注意*概率测度的转换！），并计算 $w_1 = p_1 / (p_1+p_2)$；
- 对于打到非退化 BRDF 的光线，先进行 NEE 采样，有 NEE 采样概率 $p_2$。我们计算该交点若用 BRDF 采样到 NEE 对应点的概率 $p_1$，并计算 $w_2 = p_2 / (p_1+p_2)$。

#subpar.grid(
  columns: (1fr, 1fr, 1fr),
  figure(
    image("fig/mis_test_96spp_37s_naive.png"), 
    caption: "Naive, 96spp"
  ),
  figure(
    image("fig/mis_test_16spp_9s_nee.png"), 
    caption: "NEE, 16spp"
  ),
  figure(
    image("fig/mis_test_16spp_9s_mis.png"), 
    caption: "MIS, 16spp"
  ),
  caption: "BRDF 采样与 NEE 相结合的 MIS 采样效果对比",
  label: <fig:mis_test>
)

如图@fig:mis_test 所示，MIS 采样在蓝、红两处小光源处，与 NEE 效果接近；在淡黄色大光源处，黑色大方差噪点较 NEE 更少，接近更高 spp 数的 BRDF 采样效果。将一张 Naive, 8192spp 的图片作为真实值，分别计算 NEE 和 MIS 的方差，得到 NEE 方差为 524.6，MIS 方差为 454.4，进一步说明 MIS 采样的方差更小，收敛更快。

*需要注意*，我们做加权处理的必须是同一深度上的积分。对于打到光源的光线，做的加权其实是上一深度的积分；对于打到非退化 BRDF 的光线，做的加权是当前深度的积分。而对于某一点来说，打到光源的 BRDF 部分正是到下一循环时完成的，因此该点的两部分加权的确是在同一深度。

== 基于 SAH 的 BVH 加速
#v(3pt)
BVH 是一种空间划分的加速结构，通过递归地将场景划分为一棵二叉树，使得光线与场景求交的期望复杂度降低为 $O(log n)$。我们采用 SAH（Surface Area Heuristic）作为划分的标准，即：
- 对于每个轴，将场景中所有物体归入等距划分的 $B$ 个桶中，计算每个桶的 AABB 包围盒；
- 枚举桶的前缀，分别将前缀和后缀合并为大的 AABB 包围盒，并按照 $C = C_"trav" + C_"isect" (A_"left")/A + C_"isect" (A_"right")/A$ 计算 SAH 代价。实现中，认为 $C_"trav"$ 是常数，$C_"isect"$ 正比于 AABB 包围盒所含物体个数，$A$ 为 AABB 包围盒的表面积。约去公共项后，最后的代价函数为：
$ C = \#"left" dot.c S_"left" + \#"right" dot.c S_"right" $
- 找到最小代价的划分方式，递归地划分左右子树。
在之后的光线求交中，若光线与某节点的 AABB 包围盒都不相交，则直接剪枝该子树。

最终提交图片共有 138305 个三角面片，在服务器 128 线程并行下，不使用 BVH 每 spp 用时约 30h，使用 BVH 则仅需 1.09s，得到了约 $10^5$ 倍的提速。

== 其它小项

=== 具有 Fresnel 效应的玻璃材质

在折射材质界面上，如果不是全反射，光线在折射的同时会有一部分被反射，这个现象称为 Fresnel 效应。光线发生反射的比例称为反射率，可以通过 #link("https://en.wikipedia.org/wiki/Schlick%27s_approximation")[Schlick's approximation] 近似计算。

#subpar.grid(
  columns: (1fr, 1fr),
  figure(
    image("fig/no_fresnel_CMP_checkerboard_one_ball_8192spp_471s_mis.png"), 
    caption: "无 Fresnel 效应的玻璃球",
  ), <fig:no_fresnel>,
  figure(
    image("fig/checkerboard_two_balls_8192spp_520s_mis.png"), 
    caption: "左侧水球，右侧玻璃球，均具有 Fresnel 效应"
  ), <fig:fresnel_cmp>,
  caption: "实现 Fresnel 效应与否的对比",
)

图@fig:no_fresnel 中是一个折射率 $n=1.5$ 的玻璃球，未实现 Fresnel 效应。由于光线在折射时没有反射，一方面，从视线出发进入玻璃球下方部位的光线在内部全反射而始终无法射出，导致渲染为全黑；另一方面，玻璃球的上方部位也完全没有光泽。

图@fig:fresnel_cmp 中是一个折射率 $n=1.33$ 的水球和一个折射率 $n=1.5$ 的玻璃球，均实现了 Fresnel 效应。由于光线在折射时有一部分被反射，两球的下方部位成为了一个镜面，反射出下方的棋盘格；同时，两球的上方部位具有透亮感，反射出上方的光源以及折射出后方的棋盘格。玻璃球的折射率比水球大，其全反射的临界角更小，因此玻璃球的下方“镜面”部位更大，而水球的上方部位对后方的扭曲放大作用更强。


=== 法向插值与法向贴图

对于网格模型，我们需要在三角面片上插值法向，以得到更加平滑的着色效果。网格模型一般由 OBJ 文件导入，若其中包含顶点的法向信息，则直接使用；否则，将每个顶点的法向设置为相邻面片的法向按面积加权平均。对于光线与三角面片的交点，使用重心坐标插值法向。

法向贴图则直接从法向纹理图上采样得到法向。*唯一需要注意的是*，法向贴图的法向是在纹理坐标系下的，因此在构建三角面片时，根据顶点坐标及纹理坐标预先计算从纹理坐标到世界坐标的变换矩阵（TBN 矩阵）。最终提交图片中棋盘表面的细致纹理就是通过法向贴图实现的。

=== 纹理贴图

对于网格模型，我们支持简单的纹理贴图。对于每个三角面片，我们将纹理坐标插值到交点处，然后根据纹理坐标在纹理图上的位置，进行双线性插值得到纹理颜色。

=== 景深

将原本针孔摄像机的一点扩展为一个微小的光圈。采样时，先从光圈中心射出光线，根据焦距确定与焦平面的交点，然后再在光圈内随机采样一个点，射向焦平面上的交点。

#figure(
  image("fig/cow_and_bunny_8192spp_1405s_mis.png", width: 60%),
  caption: [两只铜兔子和一头奶牛。可以看出法向插值、纹理贴图、景深、抗锯齿等效果。]
)

=== 抗锯齿

采样相机光线时，对像素中心上下、左右各 $plus.minus 0.5$ 范围内随机偏移，从而实现简单的抗锯齿效果。

=== OpenMP 加速

渲染一张图片时，各个 spp 之间、同一 spp 中各个像素点的采样都是完全独立的，天然支持并行。我们使用 OpenMP 对渲染过程进行加速，使用 `#pragma omp parallel for` 并行处理采样。

=== Gamma 校正

渲染总是在线性空间（LinearRGB）中处理颜色。我们在最终输出时将颜色转换到 sRGB 空间，以更好的符合人眼对颜色的感知。Gamma 校正的具体公式可参见 #link("https://en.wikipedia.org/wiki/SRGB#Transformation")[Wikipedia]。


= 代码逻辑
#v(5pt)

`run_all.sh`：启动脚本，内含所有测试用例的构建与运行命令

#v(3pt)

`src/`

#stack(
  dir: ltr,
  [#math.space],
  [
    - `main_whitted.cpp`：Whitted-Style 光线追踪的主函数

    - `main_pathtracing.cpp`：路径追踪的主函数

    - `shader/`：着色器，包含 Whitted-Style 光线追踪的着色器 `shader_whitted.cpp` 以及路径追踪的三种着色器 `shader_naive.cpp`、`shader_nee.cpp`、`shader_mis.cpp`

    - `scene_parser.cpp`：用于解析场景，头文件在 `../include/scene_parser.hpp`

    - `image.cpp`：用于存储渲染结果，头文件在 `../include/image.hpp`
  ]
)

#v(3pt)

`include/`

#stack(
  dir: ltr,
  [#math.space],
  [
    - `objects/`：几何形体

    - `material/`：材质

    - `sampler/`：采样器，包括均匀采样器以及为每种 BRDF 材质专门设计的采样器

    - `accel/`：加速结构，包括 AABB 包围盒以及 BVH 层次结构

    - `texture.hpp`：用于纹理贴图、法向贴图

    - `camera.hpp`, `hit.hpp`, `light.hpp`, `ray.hpp`：如名字所示

    - `utils.hpp`：一些工具函数
  ]
)


= 参考资料

== 参考资料
#v(3pt)

使用的第三方库：

- #link("https://github.com/tinyobjloader/tinyobjloader")[tinyobjloader]

- #link("https://github.com/nothings/stb")[stb_image]

系列性的参考资料：

- #link("https://sites.cs.ucsb.edu/~lingqi/teaching/games101.html")[GAMES101] - Ling-Qi Yan

- #link("https://www.pbr-book.org/3ed-2018/contents")[PBR book] (Physically Based Rendering: From Theory To Implementation)

- #link("https://www.cg.tuwien.ac.at/courses/Rendering/VU")[Rendering] - Vienna University of Technology

零散的参考注明在代码中。

== 代码原创性说明
#v(3pt)

提交代码中，注释明显的分为中英文两部分：

- 英文部分是正常的代码注释，给自己看的；

- 中文注释则是最后统一写的代码原创性说明。每个代码文件的开头都有整体的原创性标注，若内部有不同原创性的部分则会单独标注。


= 最终提交图片
#v(3pt)

#figure(
  image("fig/go_8192spp_8956s_2048_glass.png"),
  caption: [
    最终提交图片：第 29 届 LG 杯第二局棋谱
  ]
)

2025 年 1 月第 29 届 LG 杯决赛，柯洁九段对卞相壹九段弈于韩国首尔。上图为第二局棋谱，柯洁执白，卞相壹执黑。局中，柯洁因两次未将提子置于棋盒盖内而被直接判负，这是围棋史上首次、也是唯一一次犯规负。本图整体打光偏昏暗，两颗被提黑子置于阴影及景深模糊处，喻指犯规负的无趣、无聊#footnote[韩国规则下，死子数目对于局面分析是必要的。我完全认可死子需放入棋盒盖的规定，但究竟该提醒还是罚目，值得商榷。两次犯规直接判负，判罚明显过重。第三局中面对同样问题，裁判不及时介入而是等到卞长考时介入，更是不讲道理。]，与围棋这项运动本身的意蕴格格不入。画面高光及焦平面给在了最后一手白子以及棋盘仍剩余的大量空白处，意指棋局明明还有万般变化，却只留给后人一个不完整的棋谱。_围棋的胜负，就应该留在棋盘之上。_


本图分辨率 $3328 times 2048$，服务器 128 线程并行，共采样 8192spp，渲染时间 8956 秒。

本图体现的效果有：
- BVH 加速
- MIS 采样
- 多种 BRDF 材质（Lambert、Cook-Torrance、玻璃等）
- 法向插值（每颗围棋）、法向贴图（棋盘表面）、纹理贴图、景深、抗锯齿等

本图使用的部分 obj 文件是在 #link("https://sketchfab.com/3d-models/go-ghess-board-4k-materialtest-e7c17918e0314d7f8dfd385a71141e27")[Go Ghess Board - chiwei] 以及 #link("https://sketchfab.com/3d-models/go-board-game-18914af1fce84f939dafcd868aa610da")[Go Board Game - Jzax] 基础上通过 Blender 修改而来。原 3D 模型均采用 #link("https://creativecommons.org/licenses/by/4.0/")[CC BY 4.0] 协议，特此说明。经我修改后的 obj 文件仍采用 CC BY 4.0 协议。
