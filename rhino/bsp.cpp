#include <QtWidgets>
#include "gui.h"

#include "qp_port.h"
#include "pelican.h"
#include "bsp.h"

Q_DEFINE_THIS_FILE

static uint8_t const l_time_tick = 0U;                               // for QS

void QP::QF_onClockTick(void) {
    QP::QF::TICK_X(0U, &l_time_tick);
}

void QP::QF::onStartup(void) {
    QP::QF_setTickRate(BSP_TICKS_PER_SEC);
    QS_OBJ_DICTIONARY(&l_time_tick);
}

void Q_onAssert(char const * const file, int line) {
    QMessageBox::critical(0, "PROBLEM",
        QString("<p>Assertion failed in module <b>%1</b>,"
                "line <b>%2</b></p>")
            .arg(file)
            .arg(line));
    QS_ASSERTION(file, line);       // send the assertion info to the QS trace
    qFatal("Assertion failed in module %s, line %d", file, line);
}


void BSP_init(void) {
    Q_ALLEGE(QS_INIT((char *)0));
    QS_OBJ_DICTIONARY(&l_time_tick);
}

void BSP_terminate(int16_t result) {
    qDebug("terminate");
    QP::QF::stop();                                // stop the QF_run() thread
    qApp->quit();  // quit the Qt application *after* the QF_run() has stopped
}

#ifdef Q_SPY

#include "qspy.h"

static QTime l_time;

static int custParserFun(QSpyRecord * const qrec) {
    int ret = 1;                              // perform standard QSPY parsing
    switch (qrec->rec) {
        case QP::QS_QF_MPOOL_GET: {                 // example record to parse
            int nFree;
            (void)QSpyRecord_getUint32(qrec, QS_TIME_SIZE);
            (void)QSpyRecord_getUint64(qrec, QS_OBJ_PTR_SIZE);
            nFree = (int)QSpyRecord_getUint32(qrec, QF_MPOOL_CTR_SIZE);
            (void)QSpyRecord_getUint32(qrec, QF_MPOOL_CTR_SIZE);       // nMin
            if (QSpyRecord_OK(qrec)) {
                ret = 0;                // don't perform standard QSPY parsing
            }
            break;
        }
    }
    return ret;
}

bool QP::QS::onStartup(void const *arg) {
    static uint8_t qsBuf[4*1024];                 // 4K buffer for Quantum Spy
    initBuf(qsBuf, sizeof(qsBuf));

    QSPY_config(QP_VERSION,         // version
                QS_OBJ_PTR_SIZE,    // objPtrSize
                QS_FUN_PTR_SIZE,    // funPtrSize
                QS_TIME_SIZE,       // tstampSize
                Q_SIGNAL_SIZE,      // sigSize,
                QF_EVENT_SIZ_SIZE,  // evtSize
                QF_EQUEUE_CTR_SIZE, // queueCtrSize
                QF_MPOOL_CTR_SIZE,  // poolCtrSize
                QF_MPOOL_SIZ_SIZE,  // poolBlkSize
                QF_TIMEEVT_CTR_SIZE,// tevtCtrSize
                (void *)0,          // matFile,
                (void *)0,
                &custParserFun);    // customized parser function

    l_time.start();                 // start the time stamp

    //QS_FILTER_OFF(QS_ALL_RECORDS);
    QS_FILTER_ON(QS_ALL_RECORDS);

    //QS_FILTER_OFF(QS_QEP_STATE_EMPTY);
    //QS_FILTER_OFF(QS_QEP_STATE_ENTRY);
    //QS_FILTER_OFF(QS_QEP_STATE_EXIT);
    //QS_FILTER_OFF(QS_QEP_STATE_INIT);
    //QS_FILTER_OFF(QS_QEP_INIT_TRAN);
    //QS_FILTER_OFF(QS_QEP_INTERN_TRAN);
    //QS_FILTER_OFF(QS_QEP_TRAN);
    //QS_FILTER_OFF(QS_QEP_IGNORED);
    //QS_FILTER_OFF(QS_QEP_UNHANDLED);

    QS_FILTER_OFF(QS_QF_ACTIVE_ADD);
    QS_FILTER_OFF(QS_QF_ACTIVE_REMOVE);
    QS_FILTER_OFF(QS_QF_ACTIVE_SUBSCRIBE);
    //QS_FILTER_OFF(QS_QF_ACTIVE_UNSUBSCRIBE);
    //QS_FILTER_OFF(QS_QF_ACTIVE_POST_FIFO);
    //QS_FILTER_OFF(QS_QF_ACTIVE_POST_LIFO);
    QS_FILTER_OFF(QS_QF_ACTIVE_GET);
    QS_FILTER_OFF(QS_QF_ACTIVE_GET_LAST);
    QS_FILTER_OFF(QS_QF_EQUEUE_INIT);
    QS_FILTER_OFF(QS_QF_EQUEUE_POST_FIFO);
    QS_FILTER_OFF(QS_QF_EQUEUE_POST_LIFO);
    QS_FILTER_OFF(QS_QF_EQUEUE_GET);
    QS_FILTER_OFF(QS_QF_EQUEUE_GET_LAST);
    //QS_FILTER_OFF(QS_QF_MPOOL_INIT);
    //QS_FILTER_OFF(QS_QF_MPOOL_GET);
    QS_FILTER_OFF(QS_QF_MPOOL_PUT);
    //QS_FILTER_OFF(QS_QF_PUBLISH);
    QS_FILTER_OFF(QS_QF_NEW);
    QS_FILTER_OFF(QS_QF_GC_ATTEMPT);
    QS_FILTER_OFF(QS_QF_GC);
    QS_FILTER_OFF(QS_QF_TICK);
    QS_FILTER_OFF(QS_QF_TIMEEVT_ARM);
    QS_FILTER_OFF(QS_QF_TIMEEVT_AUTO_DISARM);
    QS_FILTER_OFF(QS_QF_TIMEEVT_DISARM_ATTEMPT);
    QS_FILTER_OFF(QS_QF_TIMEEVT_DISARM);
    QS_FILTER_OFF(QS_QF_TIMEEVT_REARM);
    QS_FILTER_OFF(QS_QF_TIMEEVT_POST);
    QS_FILTER_OFF(QS_QF_CRIT_ENTRY);
    QS_FILTER_OFF(QS_QF_CRIT_EXIT);
    QS_FILTER_OFF(QS_QF_ISR_ENTRY);
    QS_FILTER_OFF(QS_QF_ISR_EXIT);

    return true;                                                    // success
}

void QP::QS::onCleanup(void) {
    QSPY_stop();
}

void QP::QS::onFlush(void) {
    uint16_t nBytes = 1024;
    uint8_t const *block;
    while ((block = getBlock(&nBytes)) != (uint8_t *)0) {
        QSPY_parse(block, nBytes);
        nBytes = 1024;
    }
}

QP::QSTimeCtr QP::QS::onGetTime(void) {
    return (QSTimeCtr)l_time.elapsed();
}

void QP::QS_onEvent(void) {
    uint16_t nBytes = 1024;
    uint8_t const *block;
    QF_CRIT_ENTRY(dummy);
    if ((block = QS::getBlock(&nBytes)) != (uint8_t *)0) {
        QF_CRIT_EXIT(dummy);
        QSPY_parse(block, nBytes);
    }
    else {
        QF_CRIT_EXIT(dummy);
    }
}

void QSPY_onPrintLn(void) {
    qDebug(QSPY_line);
}

#endif                                                                // Q_SPY
