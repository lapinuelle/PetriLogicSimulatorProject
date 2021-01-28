#if !defined(GATE_HEADER)
#define GATE_HEADER

#include "internal/IGate.h"

class gate : public IGate {
protected:
	std::vector <net*> ins;               ///< ����� �������
	std::vector<LogicLevel> ins_temp;     ///< ��������� (����������) ����� ��� �������� �����
	std::vector <net*> outs;              ///< ������ �������
	std::vector<LogicLevel> outs_temp;    ///< ��������� (����������) ������ ��� �������� �����
	std::string realName;                     ///< ��� �������
	std::string name;                     ///< ��� �������
	int repeat;                          ///< ���������� ���������� ������������� ������� � ���������� ������ �������
	float delay;                            ///< �������� �������
	std::vector<std::string> tokens;
	std::map<std::string, int> jumps;
	
	// ������������ ����� IGate
public:
	void t_minus() override;
	void operate() override;
	bool t_plus() override;
	bool postprocess() override;
	void setDelay(float delay) override;
	float getDelay() override;
	void setName(std::string name) override;
	std::string getName() override;
	void addInput(net *) override;
	std::vector<net*> getInputs() override;
	void addOutput(net *) override;
	std::vector<net*> getOutputs() override;


	// ������������ ����� IGate
	virtual void setInternalInputValue(int, LogicLevel) override;

	virtual LogicLevel getInternalInputValue(int) override;


	// ������������ ����� IGate
	virtual void setInternalOutputValue(int, LogicLevel) override;

	virtual LogicLevel getInternalOutputValue(int) override;


	// ������������ ����� IGate
	virtual void setRealName(std::string name) override;

	virtual std::string getRealName() override;


	// ������������ ����� IGate
	virtual int getRepeatCount() override;


	// ������������ ����� IGate
	virtual void setRepeatCount(int) override;


	// ������������ ����� IGate
	virtual std::vector<std::string> getTokens() override;

	virtual void addToken(std::string) override;

	virtual void addJump(std::string, int) override;

	virtual int getTokensCount() override;


	// ������������ ����� IGate
	virtual std::map<std::string, int> getJumps() override;

};

#endif