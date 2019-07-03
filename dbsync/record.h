#ifndef RECORD_H
#define RECORD_H

#define UNUPLOAD 0x00
#define AVATAR_UPLOAD_OVER 0x01
#define SCORE_UPLOAD_OVER 0x10

#include <string>

class record {
public :
    record() :
        mi_session_id(-1),
        mi_seat_id(-1),
        ms_player_avatar(""),
        ms_player_score("")
    {}

    int mi_session_id ;
    int mi_seat_id;
    std::string ms_player_avatar;
    std::string ms_player_score;
};

#endif // RECORD_H

