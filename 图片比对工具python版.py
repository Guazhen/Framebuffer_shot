#coding:utf8
import sys
import os
import cv2
import numpy as np


def AssertImage( old_img, new_img, diff_img,check=None, nocheck=None,standard=90):
    '''
    old_img:原始的图片
    new_img:需要对比的图片
    diff_img:输出显示比对区域的图片
    standard:相似度结果合格的标准数值，默认为90，可由前台设置
    check：需要对比的区域,(左上角坐标，又下角坐标)
        check = [
            ((0,0),(0,1)),
            ((10,15),(50,70)),
        ]
    nocheck：不需要对比的区域
        nocheck = [
            ((0,0),(0,1)),
            ((10,15),(50,70)),
        ]
    当check和nocheck同时为空的时候，对比整张图片
    '''

    res = []
    deep = 3  # 检测深度，作为每块长宽的分母，deep值越大，分的块越小，检测越严格
    result = 0
    bResult = 100

    '''
    if check is 'None':
        check = []
    else:
    #将字符串str当成有效的表达式来求值并返回计算结果
        check = eval(check)
       
    if nocheck is 'None':
        nocheck = []
    else:
	#将字符串str当成有效的表达式来求值并返回计算结果
        nocheck = eval(nocheck)
    '''
    if not os.path.exists(old_img):
        err = "Image({0}) not exeist".format(old_img)
        print (err)
        raise RunActivceException(err)

    if not os.path.exists(new_img):
        err = "Image({0}) not exeist".format(new_img)
        print (err)
        raise RunActivceException(err)
	#cv2.imread() 读取图片，并显示
    cv2_old_img = cv2.imread(old_img)
    cv2_new_img = cv2.imread(new_img)
	#图像矩阵的shape属性表示图像的大小，shape会返回tuple元组，第一个元素表示矩阵行数，第二个元组表示矩阵列数，第三个元素是3，表示像素值由光的三原色组成
    oh, ow, _ = cv2_old_img.shape
    nh, nw, _ = cv2_new_img.shape

	#图片不一样大
    if oh != nh or ow != nw:
        print("The resolution of the old img is:[{0}*{1}]".format(oh, ow))
        print("The resolution of the new img is:[{0}*{1}]".format(nh, nw))
        raise RunActivceException("The resolution of the two pictures is different")
    # 此处保留原本算法，用于计算整体相似度，并给出比对区域的图片
    if nocheck is None:
        nocheck = []

    if check is None:
        compare(cv2_new_img, cv2_old_img, check, nocheck, diff_img=diff_img,oh=oh, ow=ow, nh=nh, nw=nw)
    else:
        for ((sx, sy), (ex, ey)) in check:
            compare(cv2_new_img, cv2_old_img, check, nocheck, diff_img=diff_img,
                         sx=sx, sy=sy, ex=ex, ey=ey)
    # 此处开始精确比对，深度deep默认为3
    oh /= deep  # 图片块的宽度
    ow /= deep
    nh /= deep
    nw /= deep
    output = None

    if check is None:
        for t in range(deep):
            for s in range(deep):
                bResult, output = compare(cv2_new_img, cv2_old_img, check, nocheck,
                                               t, s, oh=oh, ow=ow, nh=nh, nw=nw,standard=standard)
                res.append(100 - bResult)
    else:
        for ((sx, sy), (ex, ey)) in check:
            a = (ex - sx) / deep  # 图片块的长度
            b = (ey - sy) / deep  # 图片块的宽度
            for t in range(deep):
                for s in range(deep):
                    bResult, output = compare(cv2_new_img, cv2_old_img, check, nocheck,
                                                   t, s, sx=a * s + sx, sy=b * t + sy, ex=a * (s + 1) + sx,
                                                   ey=b * (t + 1) + sy,standard=standard)
                    res.append(100 - bResult)
    res.sort()
    final_result = sum(res[:3]) / len(res[:3])
    output = 'The image similarity is [{0}%],standard is [{1}%]'.format(final_result, standard)
    print (output)
    return final_result, [output, old_img, new_img, diff_img]


def compare( cv2_new_img, cv2_old_img, check, nocheck, t=0, s=0, diff_img=None, oh=0, ow=0, nh=0, nw=0, sx=0,
            sy=0, ex=0, ey=0,standard=90):
    bResult = 100
    result = 0
    if oh != nh or ow != nw:
        print ("The resolution of the old img is:[{0}*{1}]".format(oh, ow))
        print ("The resolution of the new img is:[{0}*{1}]".format(nh, nw))
        print ("The resolution of the two pictures is different")
	#empty 会创建一个没有使用特定值来初始化的数组
    cv2_diff_img = np.empty(cv2_old_img.shape, np.uint8)
	#初始化为255
    cv2_diff_img.fill(255)
	## 权重越大，透明度越低  标量加到每一个和  函数把两个图片融合在一起
    cv2_old_img = cv2.addWeighted(cv2_old_img, 0.4, cv2_diff_img, 0.5, 0)
    cv2_new_img = cv2.addWeighted(cv2_new_img, 0.4, cv2_diff_img, 0.5, 0)
    cv2_diff_img = cv2_old_img.copy()
	#宽
    width = cv2_old_img.shape[0]
	#高
    height = cv2_old_img.shape[1]

    if check is not None:
        tmp = 0
        # sx/=2
        # ex/=2
        continueFlag = False
	#x 方向的像素
        for i in range(sx, ex):
            if continueFlag:
                continueFlag = False
                continue
		#y方向的像素
            for j in range(sy, ey):
                for ((nosx, nosy), (noex, noey)) in nocheck:
			#不在对比范围之内
                    if (nosx < i and i < noex) and (nosy < j and j < noey):
                        # print "check point:({0}) in no check point {1}-{2}".format((i,j),(nosx,nosy),(noex,noey))
                        continueFlag = True
                        break
                if continueFlag:
                    continue

                diff = int(cv2_old_img[j, i][0]) - int(cv2_new_img[j, i][0])
                tmp += 1
                if diff < 0:
                    if _clmap(abs(diff), 10, 255) != 255:
                        result += 1
                    cv2_diff_img[j, i] = [0, _clmap(abs(diff), 10, 255), 0]
                elif diff > 0:
                    if _clmap(abs(diff), 10, 255) != 255:
                        result += 1
                    cv2_diff_img[j, i] = [0, 0, _clmap(abs(diff), 10, 255)]
                else:
                    pass
        if tmp != 0:
            bResult = float(result * 100) / float(tmp)
		#v2.rectangle(img, (x,y), (x+w,y+h), (0,255,0), 2)画出矩行
		#参数解释
		#第一个参数：img是原图
		#第二个参数：（x，y）是矩阵的左上点坐标
		#第三个参数：（x+w，y+h）是矩阵的右下点坐标
		#第四个参数：（0,255,0）是画线对应的rgb颜色
		#第五个参数：2是所画的线的宽度
        cv2.rectangle(cv2_diff_img, (sx, sy), (ex, ey), (0, 255, 0), thickness=3, lineType=16)
    else:

        for i in range(oh * t, oh * (t + 1)):
            continueFlag = False
            for j in range(ow * s, ow * (s + 1)):
                if continueFlag:
                    continueFlag = False
                    continue
                for ((nosx, nosy), (noex, noey)) in nocheck:
                    if (nosx < j and j < noex) and (nosy < i and i < noey):
                        # print "check point:({0}) in no check point {1}-{2}".format((i,j),(nosx,nosy),(noex,noey))
                        continueFlag = True
                        break
                if continueFlag:
                    continue

                diff = int(cv2_old_img[i, j][0]) - int(cv2_new_img[i, j][0])

                if diff < 0:
                    if _clmap(abs(diff), 10, 255) != 255:
                        result += 1
                    cv2_diff_img[i, j] = [0, _clmap(abs(diff), 10, 255), 0]
                elif diff > 0:
                    if _clmap(abs(diff), 10, 255) != 255:
                        result += 1
                    cv2_diff_img[i, j] = [0, 0, _clmap(abs(diff), 10, 255)]
                else:
                    pass
                    # result +=(abs(diff))

                bResult = float(result * 100) / float(oh * ow)
                cv2.rectangle(cv2_diff_img, (ow * s, oh * t), (ow * (s + 1), oh * (t + 1)), (0, 255, 0),
                              thickness=3, lineType=16)

    for ((nosx, nosy), (noex, noey)) in nocheck:
        cv2.rectangle(cv2_diff_img, (nosx, nosy), (noex, noey), (255, 0, 0), thickness=3, lineType=16)
    if diff_img is not None:
        output = 'The image general similarity is [{0}%],standard is [{1}%]'.format(100 - bResult,
                                                                                    standard)
        cv2.imwrite(diff_img, cv2_diff_img)
    else:
        output = 'The image area({0},{1}) similarity is [{2}%],standard is [{3}%]'.format(t, s, 100 - bResult,
                                                                                          standard)
    print (output)
    return bResult, output


def _clmap( v, k, upBound):
    val = v * k

    if standard == 'None':
        standard = int(90)
    else:
        standard = int(standard)

    if val > upBound:
        return val
    else:
        return upBound

if __name__ == '__main__':

    args = sys.argv[:]
    print args
    old_img = sys.argv[1]
    new_img = sys.argv[2]
    diff_img = sys.argv[3]
    check = sys.argv[4]
    nocheck = sys.argv[5]
    standard = sys.argv[6]
    print type(check)
    check = eval(check)
    nocheck = eval(nocheck)
    standard = eval(standard)


    AssertImage(old_img,new_img,diff_img, check=check, nocheck=nocheck, standard=standard)
    #AssertImage('112.png','111.png','diff.png')
