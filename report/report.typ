#import "conf.typ": *
#show: conf

#title("真实感渲染大作业  实验报告")
#author("TODO")

// - 基础项：推荐先实现Whitted-Style光线追踪，再实现路径追踪，前者的代码实现应真包含于后者。

// （1）Whitted-Style光线追踪：
//   - 材质：折射
//   - 材质：反射
//   - Shadow Ray：阴影

// （2）路径追踪：
//   - 支持面光源；无穷递归，终止策略使用Russian Routelette
//   - 基于BRDF的着色（使用蒙特卡洛积分计算着色），支持漫反射、理想折射、理想反射、glossy（光滑面）材质
//   - NEE（Next Event Estimation）

// - 高

//   - 参数曲面解析法求交
//   - 复杂网格模型及其求交加速（包围盒和层次结构）
//   - 更复杂的光线追踪方案：双向光线跟踪算法(BDPT)、ReSTIR、VCM算法等（光子映射类算法不得分）
//   - 体积光

// - 中

//   - 位移贴图
//   - 景深
//   - 运动模糊
//   - 色散

// - 低

//   - 更复杂的采样方法：cos-weighted采样，BRDF采样，MIS等
//   - 抗锯齿
//   - 纹理贴图
//   - 凹凸贴图
//   - 法线贴图
//   - 法向插值

// 此外还有构图分数等主观分数，GPU 加速也有加分。上面没有列出的额外效果只要实现并写明原理也会加分。如果实现了某种效果（例如：景深、色散、贴图等），请在最终提交的图片中体现。

// 最终提交的报告中请在【路径追踪基础实现】的三个得分点中选择一个，作实现前后的效果对比。
// 例如：
//   - 路径追踪的无穷递归和Whitted-Style的仅考虑一次漫反射之间的效果对比；
//   - 使用基于BRDF着色前后的效果对比；
//   - 使用NEE前后的收敛速度和效果对比；
//   - ……
// 言之成理即可。

= 实现功能

== Whitted-Style 光线追踪
#v(3pt)
支持仅考虑一次漫反射、理想反射、理想折射，以及阴影效果的 Whitted-Style 光线追踪。

== 路径追踪
#v(3pt)
支持面光源、无穷递归、RR 终止、BRDF 着色、NEE 的路径追踪。实现了漫反射（Lambert）、简单反射折射（理想反射、理想折射、玻璃）、光滑面（Modified Phong、Cook-Torrance）等多种 BRDF 材质。

支持按照 BRDF 重要性采样、支持 BRDF 采样与 NEE 相结合的 MIS 采样。

支持 BVH 加速。

支持法向插值、法向贴图、简单的纹理贴图、简单的景深、简单的抗锯齿。

= 原理简述

== Whitted-Style 光线追踪
#v(3pt)
对于成像平面的每个像素点，投射一束光线，碰到漫反射表面则与所有光源计算着色（Phong模型）并终止，期间检查是否被遮挡而相应乘上遮挡系数；碰到理想反射或理想折射表面则根据物理方程计算反射 / 折射光线；直到达到设定的最大深度。

#figure(
  image("fig/whitted_three_balls_one_glass.jpg", width: 65%),
  caption: "Whitted-Style 光线追踪"
)

== 路径追踪：BRDF 着色与 NEE 采样
#v(3pt)
渲染的本质是求解渲染方程
$ L_o (p, omega_o) = L_e (p, omega_o) + integral_(HH^+) L_i (p, omega_i) f(p, omega_i, omega_o) (omega_i, omega_n) dif omega_i $
其中 $f(dot.c)$ 为 BRDF 函数。对于理想反射、理想折射等表面，BRDF 函数具有 Dirac delta 分布，并可直接计算光线的下一方向，即上式积分事实退化；对于一般的表面，BRDF 函数非退化，则需要用蒙特卡洛方法计算积分。材料各不相同的反射特性对应不同的 BRDF 函数，从而使渲染结果具有符合物理世界的不同着色。

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
  caption: "直接采样与 NEE 采样的效果对比"
)

对于光源数量较少、面积较小的场景，在上半球面采样一个点的入射方向时，无论是随机采样还是根据 BRDF 形式设计的重要性采样（详见下节），采样到光源的概率较低，使渲染结果收敛较慢。NEE 采样则是在采样入射方向的同时，直接从光源采样，从而有效地加速了收敛。

在实现 NEE 时，有两点需要注意：
- 一是注意概率测度的变换，即在光源上采样面积微元 $dif upright(A)$ 后，通过 $dif omega = cos(theta_"light")/d^2 dif upright(A)$ 转化为上半球面的立体角微元，其中 $theta_"light"$ 为光源表面法向与光线方向的夹角，$d$ 为光源表面到交点的距离；
- 二是注意不要重复计算光源贡献，即当光线打到光源上时，若上一交点材质的 BRDF 非退化，则上一交点的 NEE 已经计算光源贡献，此时应不再考虑光源自身辐射。

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
- 二是近似认为 $(omega, omega_n) = (omega, omega_r)$，采样 $theta = arccos (1-u_1)^(1/(alpha+2)), phi = 2pi u_2$。

这也就是网上不同的人有不同的系数，且各执一词的原因。实践中，人眼几乎看不出两者的差异。

实现中，我们以 $P = k_s / (k_s + k_d)$ 的概率用上述高光采样，以 $1-P$ 的概率用 cosine-weighted 采样。

=== Cook-Torrance 材质
#v(3pt)
Cook-Torrance 的 BRDF 函数 $f = k_d 1/pi + k_s (F(omega_o, omega_n)G(omega_i, omega_o, omega_n)D(omega_h, omega_n))/(4(omega_i, omega_n)(omega_o, omega_n))$。其中 $omega_h = (omega_i+omega_o)/norm(omega_i+omega_o)$ 为半程向量；$F$ 为 Fresnel 方程，实现中采用 Fresnel-Schlick 近似；$G$ 为几何项，实现中采用 Smith 近似以及 Schlick-GGX 方程；$D$ 为法线分布函数，实现中采用 GGX 分布
$ D_"GGX" (omega_h, omega_n) = alpha^2/(pi ((omega_h, omega_n)^2 (alpha^2-1)+1)^2) $
我们设计 $p(omega) prop D_"GGX" (omega_h, omega_n) (omega_i, omega_n)$，这和 Modified Phong 一样无法直接处理，但第一种近似方式还是求不出解析形式，因此我们采用第二种近似方式 $p(omega) approx D_"GGX" (omega_h, omega_n)(omega_h, omega_n)$，注意法向分布函数需要满足的性质保证上式积分恰好为1。最后得到 $omega_h$ 在以 $omega_n$ 为轴的球坐标下的采样 $theta = arccos sqrt((1-u_1)/(u_1 (alpha^2-1)+1))), phi = 2pi u_2$。

需要特别注意，我们采样的是半程向量 $omega_h$，而不是入射方向 $omega_i$。因此最后我们不仅需要用 $omega_h$ 计算 $omega_i$，还需要变换概率密度函数 $p(omega_i) = p(omega_h) / (4 (omega_o, omega_h))$，其中 $1/(4 (omega_o, omega_h))$ 为变换的 Jacobian，并不平凡！

#figure(
  image("fig/cornell_box_balls_128spp_cmp.png"),
  caption: "Cook-Torrance 材质不同采样方式对比 (128spp)。左：uniform 采样，右：Cook-Torrance 采样"
)

== BRDF 采样与 NEE 相结合的 MIS 采样
#v(3pt)
BRDF 采样能较好处理大光源、反射集中的材质，而 NEE 能较好处理小而多的光源、远离光源的粗糙面。MIS 采样的思想是，对于两种采样方式，我们分别计算积分的估计值，然后按照两种采样方式的概率密度函数的比值加权平均，从而得到一个更好的估计值。

我们采用原论文提出的 Balanced Heuristic：对第 $i$ 种采样策略（这里 $i=1,2$），使加权后的估计值 $w_i (x) f(x)/(p_i (x))$ 与 $i$ 无关，即让所有采样策略贡献“相等”。具体地，即取 $w_i = p_i / (p_1+p_2)$。

- 对于打到光源、上一交点 BRDF 非退化的光线，已有上一步的 BRDF 采样概率 $p_1$。我们计算上一交点若用 NEE 采样到此点的概率 $p_2$（同样注意概率测度的转换！），并计算 $w_1 = p_1 / (p_1+p_2)$；
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

如图@fig:mis_test 所示，MIS 采样在蓝、红两处小光源处，与 NEE 效果接近；在淡黄色大光源处，黑色大方差噪点较 NEE 更少，接近更高 spp 数的 BRDF 采样效果。将一张 Naive, 8192spp 的图片作为真实值，分别计算 NEE 和 MIS 的方差，得到 NEE 方差为 524.6，MIS 方差为 454.4。

需要注意，我们做加权处理的必须是同一深度上的积分。对于打到光源的光线，做的加权其实是上一深度的积分；对于打到非退化 BRDF 的光线，做的加权是当前深度的积分。而对于某一点来说，打到光源的 BRDF 部分正是到下一循环时完成的，因此该点的两部分加权的确是在同一深度。

== 基于 SAH 的 BVH 加速
#v(3pt)
BVH 是一种空间划分的加速结构，通过递归地将场景划分为一棵二叉树，使得光线与场景求交的期望复杂度降低为 $O(log n)$。我们采用 SAH（Surface Area Heuristic）作为划分的标准，即：
- 对于每个轴，将场景中所有物体归入等距划分的 $B$ 个桶中，计算每个桶的 AABB 包围盒；
- 枚举桶的前缀，分别将前缀和后缀合并为大的 AABB 包围盒，并按照 $C = C_"trav" + C_"isect" (A_"left")/A + C_"isect" (A_"right")/A$ 计算 SAH 代价。实现中，认为 $C_"trav"$ 是常数，$C_"isect"$ 正比于 AABB 包围盒所含物体个数，$A$ 为 AABB 包围盒的表面积。约去公共项后，最后的代价函数为：
$ C = \#"left" dot.c S_"left" + \#"right" dot.c S_"right" $
- 找到最小代价的划分方式，递归地划分左右子树。
在之后的光线求交中，若光线与某节点的 AABB 包围盒都不相交，则直接剪枝该子树。

== 其它小项

=== 法向插值与法向贴图

对于网格模型，我们需要在三角面片上插值法向，以得到更加平滑的着色效果。网格模型一般由 OBJ 文件导入，若其中包含顶点的法向信息，则直接使用；否则，将每个顶点的法向设置为相邻面片的法向按面积加权平均。

之后，对于光线与三角面片的交点，使用重心坐标插值法向。

法向贴图则直接从法向纹理图上采样得到法向。唯一需要注意的是，法向贴图的法向是在纹理坐标系下的，因此在构建三角面片时，根据顶点坐标及纹理坐标预先计算从纹理坐标到世界坐标的变换矩阵（TBN 矩阵）。

=== 纹理贴图

对于网格模型，我们支持简单的纹理贴图。对于每个三角面片，我们将纹理坐标插值到交点处，然后根据纹理坐标在纹理图上的位置，进行双线性插值得到纹理颜色。

#figure(
  image("fig/cow_and_bunny_512spp_92s_mis.png", width: 65%),
  caption: "两只铜兔子和一头奶牛。本节的各项效果均有体现"
)

=== 景深

将原本针孔摄像机的一点扩展为一个微小的光圈。采样时，先从光圈中心射出光线，根据焦距确定与焦平面的交点，然后再在光圈内随机采样一个点，射向焦平面上的交点。

=== 抗锯齿

采样相机光线时，对像素中心上下、左右各 $plus.minus 0.5$ 范围内随机偏移，从而实现简单的抗锯齿效果。

= 最终提交图片
#v(3pt)

= 参考资料
#v(3pt)
