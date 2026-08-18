# Medicine Delivery Robot Server Prototype

This is a tiny working prototype for a hospital medicine delivery robot workflow using an ESP32 robot as a client.

## Where to deploy the server

Deploy the server on a hospital PC, Raspberry Pi, local mini-server, or private cloud VM on the hospital network. Do not deploy the main server on the ESP32 robot.

The ESP32 should run the robot control client:

- Poll the server for its assigned task.
- Move to the pharmacy.
- Wait until the pharmacist dispatches the task.
- Navigate corridors.
- Scan NFC tags near rooms.
- Confirm arrival when the scanned room matches the assigned destination.
- Release the medicine compartment after authorization.

## Run

```powershell
python server.py
```

Then open:

```text
http://localhost:3000
```

If you have Node.js and npm installed, you can also run:

```powershell
npm start
```

## Login pages

The app starts with one common login page. Demo credentials route each user to the correct workspace:

| Username | Password | Page |
|---|---|---|
| doctor | doctor123 | Doctor Page |
| pharmacy | pharmacy123 | Pharmacy Page |
| tracking | tracking123 | Tracking Page |
| robot | robot123 | Robot Page |
| history | history123 | Order History Tracking |
| admin | admin123 | Admin view for all pages |

## ESP32 WebSocket test

The admin ESP32 test page uses:

```text
ws://SERVER_IP:3000/ws/robot-chat
```

Robot heartbeat message:

```json
{
  "type": "heartbeat",
  "robotId": "RBT-001"
}
```

The heartbeat also registers the WebSocket connection as that robot, so messages sent to `RBT-001` are delivered only to the `RBT-001` connection.

Robot chat message:

```json
{
  "type": "robot_message",
  "robotId": "RBT-001",
  "message": "Hello from ESP32"
}
```

Robot order acknowledgement:

```json
{
  "type": "order_ack",
  "robotId": "RBT-001",
  "taskId": "TASK_ID_FROM_ORDER",
  "message": "Order acknowledged from ESP32"
}
```

## Prototype workflow

1. Doctor assigns medicine, patient, room number, and robot.
2. Pharmacist enters/scans the robot ID and sees the assigned patient and medicine details.
3. Pharmacist loads the medicine and marks the task as dispatched.
4. Robot polls `/api/robots/:robotId/task`.
5. Robot scans NFC room tags using `/api/robots/:robotId/nfc`.
6. When the tag maps to the destination room, the task becomes `arrived`.
7. Robot releases the compartment using `/api/tasks/:taskId/deliver`.

The dashboard includes a live hospital map with these tracking stages:

- Order placed
- Moving to pharmacy
- Order dispatched
- Medicine on wheels
- Medicine at door
- Delivered

## Example ESP32 API calls

```text
GET /api/robots/RBT-001/task
POST /api/robots/RBT-001/nfc
POST /api/tasks/<taskId>/deliver
```

NFC request body:

```json
{
  "tagId": "NFC-A101"
}
```

## Production architecture

- Server: hospital network server with database, authentication, audit logs, and dashboards.
- Doctor UI: prescription assignment.
- Pharmacist UI: scan robot QR, load medicine, dispatch.
- Robot API client: ESP32 Wi-Fi client calling server endpoints.
- Robot controller: ESP32 motor driver logic, NFC reader, obstacle sensors, battery monitor.
- Security: HTTPS, per-robot API tokens, user login, role-based access, audit trail.
- Reliability: task retry, robot heartbeat, emergency stop, low battery return-to-base.
