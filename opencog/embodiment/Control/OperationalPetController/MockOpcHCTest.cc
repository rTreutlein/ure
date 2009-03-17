/**
 * MockOpcHCTest.cc
 *
 * Author: Nil Geisweiller
 * Creation: Thu Sep 13 2007
 */

#include "MockOpcHCTest.h"
#include "SchemaMessage.h"
#include "HCTestTask.h"
#include "PetComboVocabulary.h"
#include "atom_types_init.h"

//#define LEARNING_SCHEMA "fnorb"
#define TRICK_NAME "fnorb"
std::vector<std::string> TRICK_ARGS;
#define SCHEMA_NAME "fnorb_1"
#define OWNER_NAME "Paul"
//#define PET_NAME "Fido"
#define OBJ_NAME "stick"

#define OWNER_X1 35.0
#define OWNER_Y1 35.0
#define OWNER_X2 5.5
#define OWNER_Y2 5.5
#define OWNER_X3 45.0
#define OWNER_Y3 45.0
#define PET_X 20.0
#define PET_Y 20.0
#define OBJ_X 5.0
#define OBJ_Y 5.0

#define OBJ_LENGTH 0.5
#define OBJ_WIDTH 0.5
#define OBJ_HEIGHT 0.5
#define OBJ_YAW 0.0

#define BEHAVED_STR "behaved"

#define TIME_INTERVAL 200

#define T1 0
#define T2 TIME_INTERVAL
#define T3 3*TIME_INTERVAL

#define REWARD_1 0.5
#define REWARD_2 1.0

using namespace OperationalPetController;
using namespace PetCombo;

MockOpcHCTest::MockOpcHCTest(const std::string & myId,
                             const std::string & ip,
                             int portNumber, const std::string& petId,
                             Control::SystemParameters & parameters) :
        NetworkElement (parameters, myId, ip, portNumber)
{

    opencog::atom_types_init::init();

    this->parameters = parameters;
    this->atomSpace  = new AtomSpace();
    this->spaceServer = new SpaceServer(*atomSpace);
    this->lsMessageSender = new PetMessageSender(this);

    //fill the atomSpace with the initial scene
    owner_h = atomSpace->addNode(SL_AVATAR_NODE, OWNER_NAME);
    pet_h = atomSpace->addNode(SL_PET_NODE, petId);
    obj_h = atomSpace->addNode(SL_OBJECT_NODE, OBJ_NAME);
    speed_h = atomSpace->addNode(NUMBER_NODE,
                                 boost::lexical_cast<string>(2));

    spaceServer->addSpaceInfo(true, pet_h, T1, PET_X, PET_Y,
                              OBJ_LENGTH, OBJ_WIDTH, OBJ_HEIGHT, OBJ_YAW);
    spaceServer->addSpaceInfo(true, obj_h, T1, OBJ_X, OBJ_Y,
                              OBJ_LENGTH, OBJ_WIDTH, OBJ_HEIGHT, OBJ_YAW);
    spaceServer->addSpaceInfo(true, owner_h, T1, OWNER_X1, OWNER_Y1,
                              OBJ_LENGTH, OBJ_WIDTH, OBJ_HEIGHT, OBJ_YAW);
    spaceServer->addSpaceInfo(true, owner_h, T2, OWNER_X2, OWNER_Y2,
                              OBJ_LENGTH, OBJ_WIDTH, OBJ_HEIGHT, OBJ_YAW);
    spaceServer->addSpaceInfo(true, owner_h, T3, OWNER_X3, OWNER_Y3,
                              OBJ_LENGTH, OBJ_WIDTH, OBJ_HEIGHT, OBJ_YAW);
    //add necessary nodes to represent BDs
    behaved_h = atomSpace->addNode(PREDICATE_NODE, BEHAVED_STR);
    //fill atomSpace with actions goto_obj grab and wag
    //goto_obj
    goto_obj_h = atomSpace->addNode(GROUNDED_SCHEMA_NODE,
                                    instance(id::goto_obj)->get_name().c_str());
    //grab
    grab_h = atomSpace->addNode(GROUNDED_SCHEMA_NODE,
                                instance(id::grab)->get_name().c_str());
    //wag
    wag_h = atomSpace->addNode(GROUNDED_SCHEMA_NODE,
                               instance(id::wag)->get_name().c_str());
    //add the concept of the trick
    trick_h = atomSpace->addNode(CONCEPT_NODE, TRICK_NAME);
    //add AtTimeLink to it
    tt1_h = atomSpace->addTimeInfo(trick_h, Temporal(0,
                                                     3 * TIME_INTERVAL));
    //add first behavior, goto to stick and wag the taile (subject : owner)
    //(yes the owner wag its taile)
    //for goto_obj
    HandleSeq goto_obj_seq;
    goto_obj_seq.push_back(owner_h);
    goto_obj_seq.push_back(goto_obj_h);
    goto_obj_seq.push_back(obj_h);
    goto_obj_seq.push_back(speed_h);
    Handle argl_h = atomSpace->addLink(LIST_LINK, goto_obj_seq);
    HandleSeq ev_goto_seq;
    ev_goto_seq.push_back(behaved_h);
    ev_goto_seq.push_back(argl_h);
    eval_goto_obj_h = atomSpace->addLink(EVALUATION_LINK, ev_goto_seq);
    //add atTimeLink to it
    ebd1_h = atomSpace->addTimeInfo(eval_goto_obj_h,
                                    Temporal(0, TIME_INTERVAL));
    //for grab
    HandleSeq grab_seq;
    grab_seq.push_back(owner_h);
    grab_seq.push_back(grab_h);
    grab_seq.push_back(obj_h);
    Handle arglg_h = atomSpace->addLink(LIST_LINK, grab_seq);
    HandleSeq ev_grab_seq;
    ev_grab_seq.push_back(behaved_h);
    ev_grab_seq.push_back(arglg_h);
    eval_grab_obj_h = atomSpace->addLink(EVALUATION_LINK, ev_grab_seq);
    //for wag
    HandleSeq wag_seq;
    wag_seq.push_back(owner_h);
    wag_seq.push_back(wag_h);
    Handle arglw_h = atomSpace->addLink(LIST_LINK, wag_seq);
    HandleSeq ev_wag_seq;
    ev_wag_seq.push_back(behaved_h);
    ev_wag_seq.push_back(arglw_h);
    eval_wag_h = atomSpace->addLink(EVALUATION_LINK, ev_wag_seq);
    //add atTimeLink to it
    ebdg_h = atomSpace->addTimeInfo(eval_grab_obj_h,
                                    Temporal(2 * TIME_INTERVAL,
                                    3 * TIME_INTERVAL));
    //add member link to trick concept
    HandleSeq m1_seq;
    m1_seq.push_back(ebd1_h);
    m1_seq.push_back(tt1_h);
    atomSpace->addLink(MEMBER_LINK, m1_seq);
    HandleSeq m2_seq;
    m2_seq.push_back(ebdg_h);
    m2_seq.push_back(tt1_h);
    atomSpace->addLink(MEMBER_LINK, m2_seq);

    first_try = true;
}

MockOpcHCTest::~MockOpcHCTest()
{
}

/* --------------------------------------
 * Private Methods
 * --------------------------------------
 */

/* --------------------------------------
 * Public Methods
 * --------------------------------------
 */
AtomSpace& MockOpcHCTest::getAtomSpace()
{
    return *atomSpace;
}

Pet & MockOpcHCTest::getPet()
{
    return *pet;
}

bool MockOpcHCTest::processNextMessage(MessagingSystem::Message *msg)
{
    MAIN_LOGGER.log(LADSUtil::Logger::DEBUG, "DEBUG - OPC - Received msg");

    std::cout << "OPC RECEIVED MSG" << std::endl;

    // message not for the OPC
    if (msg->getTo() != getID()) {
        return false;
    }

    // message from learning server
    if (msg->getFrom() == parameters.get("LS_ID")) {
        LearningServerMessages::SchemaMessage* sm
        = (LearningServerMessages::SchemaMessage *)msg;

        switch (sm->getType()) {
            // note: assuming arity==0 for now - Moshe

        case MessagingSystem::Message::SCHEMA:
            std::cout << "SCHEMA" << std::endl;
            std::cout << "SCHEMA NAME : " << sm->getSchemaName() << std::endl;
            std::cout << "COMBO SCHEMA : " << sm->getComboSchema() << std::endl;
            //TODO
            // add schema to combo repository and GOAL into GAI (no boost)
            //procedureRepository->add(ComboProcedure(sm->getSchemaName(), 0,
            //          sm->getComboSchema()));

            //gai->addSchema(sm->getSchemaName());
            break;

        case MessagingSystem::Message::CANDIDATE_SCHEMA:
            std::cout << "CANDIDATE_SCHEMA" << std::endl;
            std::cout << "SCHEMA NAME : " << sm->getSchemaName() << std::endl;
            std::cout << "COMBO SCHEMA : " << sm->getComboSchema() << std::endl;

            if (first_try) {
                //set up second exemplar to be sent later
                //add the concept of the trick
                //add AtTimeLink to it
                Handle tt2_h = atomSpace->addTimeInfo(trick_h,
                                                      Temporal(10 * TIME_INTERVAL,
                                                               13 * TIME_INTERVAL));
                //for goto_obj
                //add atTimeLink to it
                Handle ebd2_h = atomSpace->addTimeInfo(eval_goto_obj_h,
                                                       Temporal(10 * TIME_INTERVAL,
                                                                11 * TIME_INTERVAL));
                //for grab
                //add atTimeLink to it
                Handle ebdw_h = atomSpace->addTimeInfo(eval_wag_h,
                                                       Temporal(12 * TIME_INTERVAL,
                                                                13 * TIME_INTERVAL));
                //add member link to trick concept
                HandleSeq m1_seq;
                m1_seq.push_back(ebd2_h);
                m1_seq.push_back(tt2_h);
                atomSpace->addLink(MEMBER_LINK, m1_seq);
                HandleSeq m2_seq;
                m2_seq.push_back(ebdw_h);
                m2_seq.push_back(tt2_h);
                atomSpace->addLink(MEMBER_LINK, m2_seq);

                lsMessageSender->sendReward(TRICK_NAME, TRICK_ARGS,
                                            SCHEMA_NAME, REWARD_1);
                _HCTt->setWait2();
                first_try = false;
            } else {
                lsMessageSender->sendReward(TRICK_NAME, TRICK_ARGS,
                                            SCHEMA_NAME, REWARD_2);
                _HCTt->setWait4();
            }
            break;

        default:
            MAIN_LOGGER.log(LADSUtil::Logger::ERROR,
                            "Not a SCHEMA or CANDIDATE_SCHEMA message!!!");
            break;
        }
    }
    return false;
}

void MockOpcHCTest::setUp()
{
    _HCTt = new HCTestTask(TRICK_NAME, TRICK_ARGS,
                           OWNER_NAME, OWNER_NAME,
                           spaceServer, lsMessageSender);
    plugInIdleTask(_HCTt, 1);
}
