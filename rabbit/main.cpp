#include "gui.h"
#include "qp_port.h"
#include "pelican.h"
#include "bsp.h"

static QP::QSubscrList l_subscrSto[PELICAN::MAX_PUB_SIG];

static QF_MPOOL_EL(QP::QEvt) l_smlPoolSto[20];

int main(int argc, char *argv[]) {
    QP::GuiApp app(argc, argv);
    Gui gui;

    gui.show();

    QP::QF::init();
    BSP_init();

    QS_OBJ_DICTIONARY(l_smlPoolSto);

    QP::QF::psInit(l_subscrSto, Q_DIM(l_subscrSto));

    QP::QF::poolInit(l_smlPoolSto,
                sizeof(l_smlPoolSto), sizeof(l_smlPoolSto[0]));

    PELICAN::AO_Pelican->start(1U,
                 (QP::QEvt const **)0, 0U, // no queue
                 (void *)0, 0U);           // no stack

    return QP::QF::run(); // calls qApp->exec()
}
