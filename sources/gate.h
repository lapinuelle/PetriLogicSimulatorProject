#if !defined(GATE_HEADER)
#define GATE_HEADER

#include "internal/IGate.h"

class gate : public IGate {
protected:
	std::vector <net*> ins;               ///< Входы вентиля
	std::vector<LogicLevel> ins_temp;     ///< Временные (внутренние) входы для переноса меток
	std::vector <net*> outs;              ///< Выходы вентиля
	std::vector<LogicLevel> outs_temp;    ///< Временные (внутренние) выходы для переноса меток
	std::string realName;                     ///< Имя вентиля
	std::string name;                     ///< Имя вентиля
	int repeat;                          ///< Количество повторений моделирования вентиля в конкретный момент времени
	float delay;                            ///< Задержка вентиля
	std::vector<std::string> tokens;
	std::map<std::string, int> jumps;
	
	// Унаследовано через IGate
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


	// Унаследовано через IGate
	virtual void setInternalInputValue(int, LogicLevel) override;

	virtual LogicLevel getInternalInputValue(int) override;


	// Унаследовано через IGate
	virtual void setInternalOutputValue(int, LogicLevel) override;

	virtual LogicLevel getInternalOutputValue(int) override;


	// Унаследовано через IGate
	virtual void setRealName(std::string name) override;

	virtual std::string getRealName() override;


	// Унаследовано через IGate
	virtual int getRepeatCount() override;

};

#endif