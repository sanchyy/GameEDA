#include "Player.hh"

/**
* Write the name of your player and save this file
* with the same name and .cc extension.
*/

#define PLAYER_NAME Creeper


// DISCLAIMER: The following Demo player is *not* meant to do anything
// sensible. It is provided just to illustrate how to use the API.
// Please use AINull.cc as a template for your player.


struct PLAYER_NAME : public Player {

    /**
    * Factory: returns a new instance of this class.
    * Do not modify this function.
    */
    static Player* factory () {
        return new PLAYER_NAME;
    }


    /*------------------
        DECLARACIONS
          GLOBALS
    ------------------*/

    static constexpr int I[8]  = {-1, -1,  0,  1,  0,  1,  0, -1 };
    static constexpr int J[8]  = { 0,  1,  1,  1, -1, -1, -1, -1 };
    static constexpr int HI[4] = { 1,  0, -1,  0};
    static constexpr int HJ[4] = { 0,  1,  0, -1};

    // --------------------------------------------

    const int S       = 0; //DOWN
    const int E       = 1; //RIGHT
    const int N       = 2; //UP
    const int W       = 3; //LEFT

    const int radius  = 5;
    const int INFINIT = 1e9;

    // --------------------------------------------

    using VE    =   vector <int>;
    using VVE   =   vector <VE>;
    using QP    =   queue <Position>;

    // --------------------------------------------

    VE                  v_soldier;
    VE                  v_helicopter;
    vector <Post>       v_posts;
    VVE                 visitats;

    /*------------------
        RANDOM STUFF
    ------------------*/

    int manhattan_distance(const Position &p1, const Position &p2) {
        return abs(p1.i - p2.i) + abs(p1.j - p2.j);
    }

    Position suma(const Position &a, const Position &b) {
        return Position(a.i+b.i,a.j+b.j);
    }

    //IT CHOOSES THE POS TO GO FIRST OF ALL, LOOKING FOR THE NEAREST POST
    Position which_post(const Position &sold) {
        Position aux = Position(-1,-1);

        for (int i = 0; i < MAX; ++i)
            for (int j = 0; j < MAX; ++j) {
                if (pos_ok(i,j)) {
                    // Get Position
                    Position temp = Position(i, j);

                    // Check if city (not my property)
                    if ((post_owner(temp.i,temp.j) != -2) && (post_owner(temp.i,temp.j) != me())) {

                        // Distance
                        if (aux.i == -1 or (manhattan_distance(temp,sold) < manhattan_distance(sold,aux)))
                            aux = temp;
                    }
                }

            }
        return aux;
    }

    //LOOKS IF  ITS A GOOD PLACE TO THROW A PARACHUTER
    bool parachuter_QuestionMark(const Position &p) {
        int x = p.i;
        int y = p.j;
        if ((what(x,y) == WATER) or (what(x,y) == MOUNTAIN)) return false;
        if (fire_time(x,y) != 0) return false;
        if (which_soldier(x,y) != 0) return false;
        return true;
    }

    //LOOKS IF  ITS A GOOD PLACE TO THROW NAPALM
    bool napalm_QuestionMark(Position pos) {
        int num_enemics = 0;
        int num_meus = 0;
        bool post = false;
        for (int i = 0; i < 5; ++i)
            for (int j = 0; j < 5; ++j) {
                Position act = {pos.i - 2 + i,pos.j - 2 + j};
                int data_soldier = which_soldier(act.i,act.j);
                int owner_post = post_owner(act.i,act.j);
                if (data_soldier <= -1) continue;
                (which_soldier(act.i,act.j) == me() ? ++num_meus : ++num_enemics);
            }
        return ((num_meus < 3 && num_enemics > num_meus)
        ||      (num_enemics  >  2*num_meus-1)
        ||      (num_meus     < 2 && post));
    }

    /*----------------
        SOLDIER
    ----------------*/

    int BFS(const Position &act, const Position &obj){

        queue <pair<Position,int>> Q;
    	Q.push({Position(),-1});
    	visitats = VVE(MAX,VE(MAX,false));
    	visitats[act.i][act.j] = true;
        int aux = 0;
        bool found = false;
    	while (!Q.empty() && !found) {
    		auto p = Q.front(); Q.pop();
            for (int i = 0; i < 8 && !found; ++i) {
                int dir = (p.second == -1 ? i : p.second);
    			Position next = suma(p.first,Position(I[i],J[i]));
                if (pos_ok(next)) {
                    if (!visitats[next.i][next.j]) {
                        if (what(next.i,next.j) != MOUNTAIN
                        &&  what(next.i,next.j) != WATER
                        &&  fire_time(next.i,next.j) == 0){
                            Q.push({next,dir});
                            visitats[next.i][next.j] = true;
                        }
                    }
                }
                if (next.i == obj.i && next.j == obj.j)
                    return p.second;
            }
    	}
        return -1;
    }

    void BFS_soldier (const Position &i_pos,const Position &f_pos, QP &qp, int id){
        visitats = VVE (MAX, VE (MAX, false));
        queue <pair <Position,queue<Position>>> q;
        q.push({i_pos,queue<Position>()});
        bool trobat = false;
        visitats[i_pos.i][i_pos.j] = true;

        while ( !q.empty() &&  !trobat ) {
            pair <Position, QP > p = q.front(); q.pop();

            for (int i = 0; i < 8 && !trobat; ++i) {
                QP route = p.second;
                Position next = suma(p.first,Position(I[i],J[i]));
                    if (! visitats[next.i][next.j]) {
                        if (pos_ok(next)
                        &&  what(next.i,next.j) != MOUNTAIN
                        &&  what(next.i,next.j) != WATER
                        &&  fire_time(next.i,next.j) == 0
                        &&  which_soldier(next.i,next.j) == 0) {
                        route.push(Position(next.i,next.j));
                        q.push({next,route});
                        visitats[next.i][next.j] = true;
                    }

                    if (next.i == f_pos.i  && next.j == f_pos.j){
                        qp = route;
                        trobat = true;
                    }
                }
            }
        }
    }

    bool enemy_near (const Position &pos) {

        for (int i = pos.i - radius; i < pos.i + radius;  ++i)
            for (int j = pos.j - radius; j < pos.j + radius; ++j) {
                if (pos_ok(i,j)) {
                    if (which_soldier(i,j) != 0 && which_soldier(i,j) == me())
                        if (data(which_soldier(pos.i,pos.j)).life >= data(which_soldier(i,j)).life)
                            return true;
                }
            }
            return false;
    }

    Position which_enemy(const Position &pos) {
        Position aux = Position(-1,-1);
        for (int i = pos.i - radius; i < pos.i + radius;  ++i)
            for (int j = pos.j - radius; j < pos.j + radius; ++j) {
                if (pos_ok(i,j)) {
                    if (which_soldier(i,j) != 0 && which_soldier(i,j) == me()) {
                        if (aux.i == -1
                        || (manhattan_distance(pos,Position(i,j)) < manhattan_distance(pos,aux)
                        &&  data(which_soldier(pos.i,pos.j)).life >= data(which_soldier(i,j)).life))
                            aux = Position(i,j);
                    }
                }

            }
            return aux;
    }

    void play_soldier(int id) {
        Position act = data(id).pos;
        Position obj = (enemy_near(act) ? which_enemy(act) : which_post(act));
        QP qp;
        BFS_soldier(act,obj,qp,id);
        Position next = qp.front();
        command_soldier(id,next.i,next.j);
        //int m = BFS(act,obj);
        //command_soldier(id,act.i + I[m],act.j + J[m]);
    }

    /*----------------
        HELICOPTER
    ----------------*/

    /*
    NAPALM AREA

    * * * * *
    * * * * *
    * * H * *
    * * * * *
    * * * * *

    */

    void play_helicopter(int id) {

        // If we can, we throw napalm.
        if (data(id).napalm == 0) {
            command_helicopter(id, NAPALM);
            return;
        }

        if (! data(id).parachuters.empty())
            throw_parachuter(id);

        // With probability 20% we turn counter clockwise,
        // otherwise we try to move forward two steps.
        int c = random(1, 3);
        command_helicopter(id, c == 1 ? COUNTER_CLOCKWISE : FORWARD2);
    }

    void throw_parachuter(int id) {
        int np = 0;
        Position x = data(id).pos;
        for (int i = -2; i < 2; ++i)
            for (int j = -2; j < 2; ++j) {
                int x_i = x.i + i;
                int x_j = x.j + j;
                if (pos_ok(x_i,x_j)
                && ! data(id).parachuters.empty()
                && parachuter_QuestionMark(Position(x_i,x_j))) {
                    command_parachuter(x_i,x_j);
                }
            }
    }

    /**
    * Play method, invoked once per each round.
    */
    virtual void play () {
        int player   = me();
        v_soldier    = soldiers(player);
        v_helicopter = helicopters(player);
        for (int i = 0; i < (int)v_helicopter.size(); ++i)
            play_helicopter(v_helicopter[i]);
        for (int i = 0; i < (int)v_soldier.size(); ++i)
            play_soldier(v_soldier[i]);

    }

};

constexpr int PLAYER_NAME::I[8];
constexpr int PLAYER_NAME::J[8];
constexpr int PLAYER_NAME::HI[4];
constexpr int PLAYER_NAME::HJ[4];

/**
* Do not modify the following line.
*/
RegisterPlayer(PLAYER_NAME);