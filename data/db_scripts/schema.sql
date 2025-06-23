-- 实验室预约系统数据库结构
PRAGMA foreign_keys = ON;  -- 启用外键约束

-- 1. 用户表（存储用户账号与权限）
CREATE TABLE users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    username TEXT UNIQUE NOT NULL,
    password TEXT NOT NULL,
    real_name TEXT NOT NULL,
    email TEXT,
    phone TEXT,
    role TEXT NOT NULL,  -- 角色：student(学生), teacher(教师), admin(管理员)
    status TEXT NOT NULL DEFAULT 'active',  -- 状态：active(激活), frozen(冻结)
    create_time TEXT NOT NULL,
    last_login_time TEXT
);

-- 2. 实验室表（存储实验室基本信息）
CREATE TABLE labs (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    location TEXT NOT NULL,
    capacity INTEGER NOT NULL,
    device_types TEXT,  -- 设备类型，逗号分隔
    description TEXT,
    open_time TEXT,  -- 开放时间，格式：08:00-22:00
    status TEXT NOT NULL DEFAULT 'available',  -- 状态：available(可用), maintenance(维护中)
    create_time TEXT NOT NULL
);

-- 3. 预约状态表（存储预约状态定义）
CREATE TABLE booking_status (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    status TEXT NOT NULL UNIQUE,
    description TEXT
);

-- 4. 预约表（存储预约记录）
CREATE TABLE bookings (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL,
    lab_id INTEGER NOT NULL,
    status_id INTEGER NOT NULL DEFAULT 1,
    booking_date TEXT NOT NULL,
    time_slot TEXT NOT NULL,  -- 时间段：morning, afternoon, evening
    use_purpose TEXT,
    apply_time TEXT NOT NULL,
    review_time TEXT,
    reviewer_id INTEGER,
    remark TEXT,
    FOREIGN KEY (user_id) REFERENCES users(id),
    FOREIGN KEY (lab_id) REFERENCES labs(id),
    FOREIGN KEY (status_id) REFERENCES booking_status(id)
);

-- 5. 实验室设备表（存储实验室设备信息）
CREATE TABLE lab_devices (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    lab_id INTEGER NOT NULL,
    device_name TEXT NOT NULL,
    model TEXT,
    quantity INTEGER NOT NULL DEFAULT 1,
    status TEXT NOT NULL DEFAULT 'available',  -- 状态：available(可用), broken(故障), reserved(已预约)
    FOREIGN KEY (lab_id) REFERENCES labs(id)
);

-- 6. 预约设备关联表（存储预约时申请的设备）
CREATE TABLE booking_devices (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    booking_id INTEGER NOT NULL,
    device_id INTEGER NOT NULL,
    quantity INTEGER NOT NULL DEFAULT 1,
    FOREIGN KEY (booking_id) REFERENCES bookings(id),
    FOREIGN KEY (device_id) REFERENCES lab_devices(id)
);
