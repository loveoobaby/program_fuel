# Flex布局
## 1.1 布局原理
通过给父盒子添加flex属性，来控制子盒子的位置与排列方式。
  + 当设置父盒子为flex布局之后，子元素的float、clear、vertical-align属性将失效；
  + 采用flex布局的元素，称为flex容器，子元素称为项目item，子元素可以横向或纵向排列

## 1.2 flex布局父项常用属性
  + flex-direction：设置主轴的方向；
  + justify-content: 设置主轴上的元素排列方式；
  + flex-wrap：设置子元素是否换行
  + align-content：设置侧轴上子元素的排列方式（多行）；
  + align-item：设置侧轴上子元素的排列方式（单行）；
  + flex-flow：复合属性，相当于同时设置了flex-direction、flex-wrap

### 1.2.1 flex-direction
   在flex布局中，是分主轴和侧轴的两个方向，默认主轴沿横向；flex-direction可以改变主轴的方向。
   |  属性值   | 说明  |
   |  ----  | ----  |
   | row           | 默认值从左到右 |
   | row-reverse   | 从右到左 |
   | column        | 从上到下 |
   | column-reverse| 从下到上 |

### 1.2.2 justify-content设置主轴上子元素的排列方式
   |  属性值   | 说明  |
   |  ----  | ----  |
   | flex-start           | 默认值，从头开始，如果主轴是X轴，则从左到右 |
   | flex-end   | 从尾部开始排列 |
   | center        | 在主轴居中对齐 |
   | space-around| 平分剩余空间 |
   | space-around| 先两边贴边，再平分剩余空间 |

### 1.2.3 flex-wrap: 控制子元素是否换行
   |  属性值   | 说明  |
   |  ----  | ----  |
   | nowrap           | 不换行 |
   | wrap   | 换行 |

### 1.2.4 align-items: 控制侧轴子元素排列方式(单行)
   |  属性值   | 说明  |
   |  ----  | ----  |
   | flex-start  | 默认值，从上到下 |
   | flex-end  | 从下到上 |
   | center    | 居中 |
   |stretch    | 拉伸|

### 1.2.4 align-content设置侧轴上的子元素的排列方式（多行）
设置侧轴上的子元素的排列方式，只在子项出现换行的情况下有效，在单行下没有效果。
   |  属性值   | 说明  |
   |  ----  | ----  |
   | flex-start  | 默认值，在侧轴的头部开始排列 |
   | flex-end  | 在侧轴尾部开始排列 |
   | center    | 居中 |
   | space-around    | 子项在侧轴平分剩余空间|
   | space-between    | 子项在侧轴先分布在两头，再平分剩余空间|
   | stretch    | 子元素高度平分父元素高度|

 ### 1.2.5 flex-flow：是flex-direction与flex-wrap的复合写法
    flex-direction: clolumn;
    flex-wrap: wrap;
    ==>等价于
    flex-flow: column wrap;

## 1.2 常见子项属性
### 1.2.1 flex属性
定义子项目分配剩余空间，用flex来表示占多少份；
.item {
  flex: <number>;
}

### 1.2.2 align-self: 控制子项自己在侧轴上的排列方式
### 1.2.3 order：定义项目的排列顺序


