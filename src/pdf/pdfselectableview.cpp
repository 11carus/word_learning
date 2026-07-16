#include "pdfselectableview.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPdfDocument>
#include <QResizeEvent>
#include <QScrollBar>
#include <QWheelEvent>

namespace {
constexpr int kPageMargin = 12;
constexpr int kWheelScrollMultiplier = 2;
constexpr qreal kMinimumZoomFactor = 0.5;
constexpr qreal kMaximumZoomFactor = 3.0;
}

PdfSelectableView::PdfSelectableView(QWidget *parent)
    : QAbstractScrollArea(parent)
{
    setMouseTracking(true);
    viewport()->setCursor(Qt::IBeamCursor);
    verticalScrollBar()->setSingleStep(48);
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, [this]() { update(); });
}

void PdfSelectableView::setDocument(QPdfDocument *document)
{
    m_document = document;
    m_currentPage = 0;
    m_pageImage = {};
    m_renderedSize = {};
    m_logicalPageSize = {};
    m_selection.reset();
    horizontalScrollBar()->setValue(0);
    verticalScrollBar()->setValue(0);
    update();
}

void PdfSelectableView::setCurrentPage(int page)
{
    if (!m_document || page < 0 || page >= m_document->pageCount() || page == m_currentPage) {
        return;
    }

    m_currentPage = page;
    m_pageImage = {};
    m_renderedSize = {};
    m_logicalPageSize = {};
    m_selection.reset();
    horizontalScrollBar()->setValue(0);
    verticalScrollBar()->setValue(0);
    emit currentPageChanged(m_currentPage);
    update();
}

int PdfSelectableView::currentPage() const
{
    return m_currentPage;
}

void PdfSelectableView::setZoomFactor(qreal factor)
{
    const qreal boundedFactor = qBound(kMinimumZoomFactor, factor, kMaximumZoomFactor);
    if (!m_fitsWidth && qFuzzyCompare(m_zoomFactor, boundedFactor)) {
        return;
    }

    m_zoomFactor = boundedFactor;
    m_fitsWidth = false;
    m_renderedSize = {};
    m_logicalPageSize = {};
    horizontalScrollBar()->setValue(0);
    verticalScrollBar()->setValue(0);
    emit zoomFactorChanged(m_zoomFactor, m_fitsWidth);
    update();
}

qreal PdfSelectableView::zoomFactor() const
{
    return m_zoomFactor;
}

void PdfSelectableView::fitToWidth()
{
    if (m_fitsWidth) {
        return;
    }

    m_zoomFactor = 1.0;
    m_fitsWidth = true;
    m_renderedSize = {};
    m_logicalPageSize = {};
    horizontalScrollBar()->setValue(0);
    verticalScrollBar()->setValue(0);
    emit zoomFactorChanged(m_zoomFactor, m_fitsWidth);
    update();
}

QString PdfSelectableView::selectedText() const
{
    return m_selection && m_selection->isValid() ? m_selection->text() : QString();
}

void PdfSelectableView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(viewport());
    painter.fillRect(viewport()->rect(), palette().color(QPalette::Mid));

    updateRenderedPage();
    if (m_pageImage.isNull()) {
        return;
    }

    painter.drawImage(m_pageRect.topLeft(), m_pageImage);
    if (!m_selection || !m_selection->isValid()) {
        return;
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(66, 133, 244, 110));
    for (const QPolygonF &polygon : m_selection->bounds()) {
        QPolygonF mapped;
        for (const QPointF &point : polygon) {
            mapped.append(viewPointAt(point));
        }
        painter.drawPolygon(mapped);
    }
}

void PdfSelectableView::resizeEvent(QResizeEvent *event)
{
    QAbstractScrollArea::resizeEvent(event);
    m_renderedSize = {};
    m_logicalPageSize = {};
    update();
}

void PdfSelectableView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton || !m_pageRect.contains(event->position())) {
        QAbstractScrollArea::mousePressEvent(event);
        return;
    }

    m_dragging = true;
    m_dragStart = pagePointAt(event->position());
    m_selection.reset();
    update();
    event->accept();
}

void PdfSelectableView::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_dragging) {
        return;
    }

    updateSelection(event->position());
    event->accept();
}

void PdfSelectableView::mouseReleaseEvent(QMouseEvent *event)
{
    if (!m_dragging || event->button() != Qt::LeftButton) {
        QAbstractScrollArea::mouseReleaseEvent(event);
        return;
    }

    updateSelection(event->position());
    m_dragging = false;
    emit textSelected(selectedText());
    event->accept();
}

void PdfSelectableView::wheelEvent(QWheelEvent *event)
{
    const int pixelDelta = event->pixelDelta().y();
    const int angleDelta = event->angleDelta().y();
    const int scrollAmount = pixelDelta != 0
        ? pixelDelta * kWheelScrollMultiplier
        : (angleDelta / 120) * verticalScrollBar()->singleStep() * kWheelScrollMultiplier;

    if (scrollAmount != 0) {
        verticalScrollBar()->setValue(verticalScrollBar()->value() - scrollAmount);
        event->accept();
        return;
    }

    QAbstractScrollArea::wheelEvent(event);
}

void PdfSelectableView::updateRenderedPage()
{
    if (!m_document || m_document->pageCount() <= 0) {
        return;
    }

    const QSizeF pagePoints = m_document->pagePointSize(m_currentPage); // PDF 页面原始点尺寸。
    const QSize available = viewport()->size() - QSize(2 * kPageMargin, 0); // 扣除页边距后的可用区域。
    if (pagePoints.isEmpty() || available.isEmpty()) {
        return;
    }

    // 先计算“适合窗口宽度”的基础比例，再叠加用户选择的缩放比例。
    // logicalSize 用于界面布局，renderSize 乘以设备像素比后用于实际渲染，
    // 从而在高 DPI 屏幕上仍能保持清晰。
    const qreal fitScale = available.width() / pagePoints.width(); // 页面适宽时的基础比例。
    const qreal scale = fitScale * m_zoomFactor; // 叠加用户缩放后的最终比例。
    const QSize logicalSize(qMax(1, qRound(pagePoints.width() * scale)), qMax(1, qRound(pagePoints.height() * scale)));
    const qreal devicePixelRatio = viewport()->devicePixelRatioF(); // 屏幕物理像素与逻辑像素之比。
    const QSize renderSize(qMax(1, qRound(logicalSize.width() * devicePixelRatio)),
                           qMax(1, qRound(logicalSize.height() * devicePixelRatio)));
    if (renderSize != m_renderedSize) {
        m_pageImage = m_document->render(m_currentPage, renderSize);
        m_pageImage.setDevicePixelRatio(devicePixelRatio);
        m_renderedSize = renderSize;
        m_logicalPageSize = logicalSize;
    }

    const int maximumScroll = qMax(0, m_logicalPageSize.height() + 2 * kPageMargin - viewport()->height());
    const int maximumHorizontalScroll = qMax(0, m_logicalPageSize.width() + 2 * kPageMargin - viewport()->width());
    verticalScrollBar()->setRange(0, maximumScroll);
    verticalScrollBar()->setPageStep(viewport()->height());
    horizontalScrollBar()->setRange(0, maximumHorizontalScroll);
    horizontalScrollBar()->setPageStep(viewport()->width());
    m_pageRect = QRectF(
        kPageMargin - horizontalScrollBar()->value(),
        kPageMargin - verticalScrollBar()->value(),
        m_logicalPageSize.width(),
        m_logicalPageSize.height());
}

QPointF PdfSelectableView::pagePointAt(const QPointF &position) const
{
    if (!m_document || m_pageRect.isEmpty()) {
        return {};
    }

    // 将鼠标所在的视图坐标线性映射为 PDF 页面的原始坐标。
    // 需要先减去页面左上角偏移，再按“页面尺寸 / 显示尺寸”缩放。
    const QSizeF pagePoints = m_document->pagePointSize(m_currentPage);
    return QPointF(
        (position.x() - m_pageRect.x()) * pagePoints.width() / m_pageRect.width(),
        (position.y() - m_pageRect.y()) * pagePoints.height() / m_pageRect.height());
}

QPointF PdfSelectableView::viewPointAt(const QPointF &position) const
{
    if (!m_document || m_pageRect.isEmpty()) {
        return {};
    }

    // pagePointAt 的逆变换：把 PDF 返回的文字选区坐标映射回视图，
    // 供 paintEvent 在正确位置绘制选区高亮。
    const QSizeF pagePoints = m_document->pagePointSize(m_currentPage);
    return QPointF(
        m_pageRect.x() + position.x() * m_pageRect.width() / pagePoints.width(),
        m_pageRect.y() + position.y() * m_pageRect.height() / pagePoints.height());
}

void PdfSelectableView::updateSelection(const QPointF &position)
{
    if (!m_document || m_pageRect.isEmpty()) {
        return;
    }

    // 把拖拽终点限制在页面矩形内，避免向页面外拖动时产生无效坐标。
    const QPointF clampedPosition(
        qBound(m_pageRect.left(), position.x(), m_pageRect.right()),
        qBound(m_pageRect.top(), position.y(), m_pageRect.bottom()));
    const QPointF endPoint = pagePointAt(clampedPosition);
    m_selection = m_document->getSelection(m_currentPage, m_dragStart, endPoint);
    update();
}
