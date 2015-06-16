#include "gui.h"

#include "qp_port.h"
#include "common.h"
#include "pelican.h"
#include "net_mgr.h"
#include "bsp.h"

//............................................................................
static QP::QSubscrList l_subscrSto[PELICAN::MAX_PUB_SIG];

static QF_MPOOL_EL(QP::QEvt) l_smlPoolSto[20]; // storage for small event pool

//............................................................................
int main(int argc, char *argv[]) {
  static QP::QEvt const * net_queue[10000];

    QP::GuiApp app(argc, argv);
    Gui gui;

    gui.show();

    QP::QF::init();  // initialize the framework
    BSP_init();      // initialize the Board Support Package

    QS_OBJ_DICTIONARY(l_smlPoolSto);

    QP::QF::psInit(l_subscrSto, Q_DIM(l_subscrSto));

    QP::QF::poolInit(l_smlPoolSto,
                sizeof(l_smlPoolSto), sizeof(l_smlPoolSto[0]));

    PELICAN::AO_Pelican->start(1U,
                 (QP::QEvt const **)0, 0U, // no queue
                 (void *)0, 0U);           // no stack

    PELICAN::AO_net_mgr->start(2U,
                         net_queue, Q_DIM(net_queue),
                         static_cast<void *>(0), 0U);

    return QP::QF::run(); // calls qApp->exec()
}
