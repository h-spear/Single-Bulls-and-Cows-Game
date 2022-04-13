#ifndef _GAME_H
#define _GAME_H

// FND�迭�� ����� 4���� ���ڸ� FND�� ���
void displayFND();

// FND �迭 �ʱ�ȭ
void clearFND();

// ���ڷ� ������ 4���� ���׸�Ʈ ���ڸ� ���÷��̿� �뷫 2�ʰ� ���
void displayFND_2s(unsigned char array[]);

// LCD�� ��������� ���� ����� �����ش�.
void showHistory();

// ���� guess�� ���� ���� ����� history�� ����
void recordHistory(int strike_cnt, int ball_cnt);

// �ð� ����
void timeUp();

// ���� �ð� �ʱ�ȭ(LED�� ����)
void timeClear();

// �ð� üũ
void timeCheck();

// 4x4 Ű�е忡�� ���ڸ� �Է¹����� �ش� index�� ��ȯ
int pushHex();

// 4-button ����ġ���� �Է¹��� ���� cmd�� ����
int pushCmd();

// ����ڰ� �Է��� ���ڸ� guess �迭�� ����
void enterGuess(int key);

// �Է¹��� cmd���� ���� �׼� ����
void cmdSwitch(int cmd);

// ����ڰ� switch�� ��带 �����ߴ��� üũ
void checkModeSwitch();

// guess �迭�� ���� answer �迭�� ���� ���Ͽ� ��� ���
void compare();

// ���� ����
void stopGame();

// ���� �ʱ�ȭ
void initGame();

// ���� ����
void progress();

// ���� ���� �Լ�
void game();

#endif
