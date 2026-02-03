using Google.Protobuf;
using Protocol;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Text;
using UnityEngine;
using UnityEngine.SceneManagement;

public static partial class PacketManager
{
    /// <summary>
    /// 최종적으로 받은 데이터를 인게임에 적용하는 함수.
    /// 싱글톤인 ServerConnect의 Instance를 통해 해당 오브젝트들에 접근해 데이터를 반영한다.
    /// </summary>
    /// <param name="type"></param>
    /// <param name="packet"></param>

    private static void Process(PacketType type, IMessage packet)
    {
        Debug.Log("받은 패킷의 type : " + type.ToString());
        switch (type)
        {
            case PacketType.PKT_S_RTT_PONG:
                S_RTT_PONG s_RTT_PONG = packet as S_RTT_PONG;

                //패킷을 받았을 때 시간
                long recvTimeMs = DateTimeOffset.UtcNow.ToUnixTimeMilliseconds();

                //패킷을 보내고 응답이 오기까지 걸린 시간
                long rtt = recvTimeMs - s_RTT_PONG.ClientTime;

                //서버가 패킷을 보낸 시간과 클라이언트가 패킷을 받은 시간의 차이
                long offset = s_RTT_PONG.ServerTime - (s_RTT_PONG.ClientTime + rtt / 2);

                UnitManager.Instance.serverOffset = offset;
                break;

            case PacketType.PKT_S_LOGIN:
                S_LOGIN s_LOGIN = packet as S_LOGIN;
                if (s_LOGIN.LoginAccept)
                {
                    Debug.Log("!!!!!로그인 성공!!!!!");
                    ServerConnect.Instance.UserId = s_LOGIN.GameId.ToString();
                    ServerConnect.Instance.SceneChange("MatchingScene");
                }
                else
                {
                    LoginManager.Instance.FailLogin();
                    Debug.Log("?????로그인 실패?????");
                }
                break;

            case PacketType.PKT_S_ROOM_LOBBY:
                S_ROOM_LOBBY s_ROOM_LOBBY = packet as S_ROOM_LOBBY;

                string roomName = s_ROOM_LOBBY.GameName;
                string roomPassWord = s_ROOM_LOBBY.GamePassWord;

                RoomData.Instance.SaveLobbyData(s_ROOM_LOBBY.HostId, roomName, roomPassWord, s_ROOM_LOBBY.MapHash.ToByteArray());
                ServerConnect.Instance.currentRoomCode = s_ROOM_LOBBY.RoomCode;
                SceneManager.LoadScene("MatchMakingScene");
                break;

            case PacketType.PKT_S_LOBBY_PLAYER_INFO:
                S_LOBBY_PLAYER_INFO s_LOBBY_PLAYER_INFO = packet as S_LOBBY_PLAYER_INFO;

                ServerConnect.Instance.playerInfo.Clear();

                foreach (Protocol.PlayerInfo playerInfo in s_LOBBY_PLAYER_INFO.PlayerData)
                {
                    PlayerInfo info = new PlayerInfo();
                    info.PlayerId = playerInfo.PlayerId;

                    ServerConnect.Instance.playerInfo.Add(info);
                }
                ServerConnect.Instance.callback?.Invoke();

                break;

            case PacketType.PKT_S_GAME_START:
                S_GAME_START s_GAME_START = packet as S_GAME_START;

                PacketRelay.Instance.MapLeader(s_GAME_START.MapSectionCount, s_GAME_START.MapData);

                SceneManager.LoadScene("BattleScene");

                break;

            case PacketType.PKT_S_ROOM_EXIT:
                S_ROOM_EXIT s_ROOM_EXIT = packet as S_ROOM_EXIT;

                RoomData.Instance.currentDisconnectCode = (DisconnectCode)s_ROOM_EXIT.DiconnectCode;

                SceneManager.LoadScene("MatchingScene");


                break;

            case PacketType.PKT_S_ROOM_DATA:
                S_ROOM_DATA s_ROOM_DATA = packet as S_ROOM_DATA;

                foreach (Protocol.RoomData data in s_ROOM_DATA.RoomData)
                {
                    string name = data.RoomName;

                    ServerConnect.Instance.showRoomInfoAction(data.RoomCode, data.PlayerCount, name, data.IsPassWord, data.MapHash.ToByteArray());
                }
                break;

            case PacketType.PKT_S_ROOM_RESPONSE:
                S_ROOM_RESPONSE s_ROOM_RESPONSE = packet as S_ROOM_RESPONSE;

                ServerConnect.Instance.callback?.Invoke();

                //if(s_ROOM_RESPONSE.RoomAccept)
                //{
                //    //UnitManager.Instance;

                //    ServerConnect.Instance.SceneChange("BattleScene");

                //    ServerConnect.Instance.currentRoomCode = s_ROOM_RESPONSE.RoomCode;

                //    foreach (Protocol.ObjectData data in s_ROOM_RESPONSE.ObjectData)
                //    {
                //        UnityEngine.Vector3 pos = new UnityEngine.Vector3(data.Position.X, data.Position.Y, data.Position.Z);

                //        UnityEngine.Vector3 dir = new UnityEngine.Vector3(data.Direction.X, data.Direction.Y, data.Direction.Z);

                //        float angle = Mathf.Atan2(dir.y, dir.x) * Mathf.Rad2Deg;

                //        ServerConnect.Instance.EnqueueSpawn((UnitCode)data.Type, data.ObjectId, pos, Quaternion.Euler(0, 0, angle));
                //    }
                //}
                //else
                //{
                //    Debug.Log("방 입장 거부됨");
                //}

                break;

            case PacketType.PKT_S_MOVE:
                S_MOVE s_MOVE = packet as S_MOVE;

                UnitManager unitManager = UnitManager.Instance;

                if (unitManager.units.TryGetValue(s_MOVE.ObjectId, out Unit unit))
                {
                    UnityEngine.Vector3 pos = new UnityEngine.Vector3(s_MOVE.Position.X, s_MOVE.Position.Y, s_MOVE.Position.Z);

                    unit.Move((GameObjectState)s_MOVE.State, pos);
                }

                break;

            case PacketType.PKT_S_OBJECT_SPAWN:
                {
                    Debug.Log("BulletSpawn");
                    S_OBJECT_SPAWN s_OBJECT_SPAWN = packet as S_OBJECT_SPAWN;

                    Protocol.ObjectData data = s_OBJECT_SPAWN.ObjectData;

                    UnityEngine.Vector3 pos = new UnityEngine.Vector3(data.Position.X, data.Position.Y, data.Position.Z);

                    Debug.Log("============== " + pos.ToString());

                    UnityEngine.Vector3 dir = new UnityEngine.Vector3(data.Direction.X, data.Direction.Y, data.Direction.Z);

                    float angle = Mathf.Atan2(dir.y, dir.x) * Mathf.Rad2Deg;

                    ServerConnect.Instance.EnqueueSpawn((UnitCode)data.Type, data.ObjectId, data.PlayerId, pos, UnityEngine.Quaternion.Euler(0, 0, angle), data.Hp, s_OBJECT_SPAWN.SpawnTime);

                    break;
                }

            case PacketType.PKT_S_OBJECT_DEAD:
                S_OBJECT_DEAD s_OBJECT_DEAD = packet as S_OBJECT_DEAD;

                if (s_OBJECT_DEAD.State == Protocol.GameObjectState.Dead)
                    UnitManager.Instance.RemoveUnit(s_OBJECT_DEAD.ObjectId);

                break;

            case PacketType.PKT_S_OBJECT_DAMAGE:
                S_OBJECT_DAMAGE s_OBJECT_DAMAGE = packet as S_OBJECT_DAMAGE;

                Unit target = UnitManager.Instance.units[s_OBJECT_DAMAGE.ObjectId];

                if (target is PlayerUnit player)
                {
                    player.SetHp(s_OBJECT_DAMAGE.Hp);
                }
                else
                {
                    target.SetHp(s_OBJECT_DAMAGE.Hp);
                }

                break;

            default:
                Debug.Log("-------------존재하지 않는 패킷------------");
                break;
        }
    }
}
