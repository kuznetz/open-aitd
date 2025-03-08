//glEnable(GL_STENCIL_TEST);
//
//Then you first draw your masking quad only in the stencil buffer :
//
////begin mask
//glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
//glStencilFunc(GL_NEVER, 0, 0);
//glStencilOp(GL_INCR, GL_KEEP, GL_KEEP);
//glClear(GL_STENCIL_BUFFER_BIT);
////draw your mask, a quad in your case
//
//Then you draw the masked part :
//
//////masked fill part
//glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
//glStencilFunc(GL_NOTEQUAL, 0, 1);
//glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
//// draw the masked element, the text in your case
//
//Disable stencil testing again
//
//glDisable(GL_STENCIL_TEST);


//// Функция для рисования треугольника по трем вершинам
//void drawTriangle(cameraPointStruct p1, cameraPointStruct p2, cameraPointStruct p3) {
//    // Упорядочить вершины по возрастанию Y
//    if (p1.y > p2.y) std::swap(p1, p2);
//    if (p1.y > p3.y) std::swap(p1, p3);
//    if (p2.y > p3.y) std::swap(p2, p3);
//
//    // Рассчитаем начальные и конечные X для каждого ряда пикселей
//    int yMin = p1.y;
//    int yMax = p3.y;
//
//    for (int y = yMin; y <= yMax; ++y) {
//        float x1, x2;
//
//        // Вычисляем пересечения слева и справа
//        if (y < p2.y) {
//            // Между p1 и p2
//            x1 = static_cast<float>(p1.x) + (static_cast<float>(y - p1.y) / (p2.y - p1.y)) * (p2.x - p1.x);
//            // Между p1 и p3
//            x2 = static_cast<float>(p1.x) + (static_cast<float>(y - p1.y) / (p3.y - p1.y)) * (p3.x - p1.x);
//        }
//        else {
//            // Между p2 и p3
//            x1 = static_cast<float>(p2.x) + (static_cast<float>(y - p2.y) / (p3.y - p2.y)) * (p3.x - p2.x);
//            // Между p1 и p3
//            x2 = static_cast<float>(p1.x) + (static_cast<float>(y - p1.y) / (p3.y - p1.y)) * (p3.x - p1.x);
//        }
//
//        // Заполняем строку пикселей между x1 и x2
//        int left = static_cast<int>(std::ceil(x1));
//        int right = static_cast<int>(std::floor(x2));
//        for (int x = left; x <= right; ++x) {
//            setPixel(x, y, 255);  // Установим белый пиксель
//        }
//    }
//}